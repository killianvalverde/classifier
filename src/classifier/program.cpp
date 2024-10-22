/* classifier
 * Copyright (C) 2024 Killian Valverde.
 *
 * This file is part of classifier.
 *
 * classifier is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * classifier is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with classifier. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file        classifier/program.cpp
 * @brief       program class implementation.
 * @author      Killian Valverde
 * @date        2024/10/15
 */

#include <fstream>

#include "json.hpp"
#include "program.hpp"


namespace classifier {


program::program(program_args&& prog_args)
        : prog_args_(std::move(prog_args))
        , inode_st_()
        , extra_files_fnd_(false)
{
}


int program::execute()
{
#if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
#endif
    escape_special_characters_in_regex_string(prog_args_.categories_file_nme);

    spd::fsys::directory_iteration source_dir_iteration(prog_args_.source_dir);
    source_dir_iteration.regex_to_match(prog_args_.categories_file_nme)
                        .file_types(spd::sys::fsys::file_types::REGULAR_FILE);

    spd::fsys::directory_iteration destination_dir_iteration(prog_args_.destination_dir);
    destination_dir_iteration.follow_symbolic_links(false)
                             .access_modes(spd::sys::fsys::access_modes::WRITE |
                                           spd::sys::fsys::access_modes::READ)
                             .regex_to_match(R"(^([^\.]*|.*\.lnk|.*\.ini)$)");

    for (auto& x : source_dir_iteration)
    {
        parse_categories_file(x);
    }

    configure_directory(prog_args_.destination_dir);

    for (auto& x : destination_dir_iteration)
    {
        check_extra_file(x);
    }

    if (extra_files_fnd_)
    {
        int inpt;

        std::cout << spd::ios::set_light_red_text
                  << "Delete all extra files? [y/N] "
                  << spd::ios::set_default_text
                  << std::flush;

        spd::sys::term::flush_input_terminal(stdin);
        inpt = getc(stdin);
        spd::sys::term::flush_input_terminal(stdin);

        if (inpt == 'y')
        {
            for (auto& x : destination_dir_iteration)
            {
                delete_extra_file(x);
            }
        }
        else
        {
            std::cout << spd::ios::set_white_text
                      << "Abort."
                      << spd::ios::set_default_text
                      << spd::ios::newl;
        }
    }

    return 0;
}


bool program::parse_categories_file(const std::filesystem::path& categories_file_pth)
{
    std::ifstream ifstr;
    json json_parsr;

    std::cout << spd::ios::set_light_cyan_text
              << "Parsing categories file: "
              << spd::ios::set_white_text
              << "\""
              << spd::cast::type_cast<std::string>(categories_file_pth.c_str())
              << "\" "
              << spd::ios::set_default_text
              << std::flush;

    ifstr.open(categories_file_pth);
    if (!ifstr.is_open())
    {
        goto error;
    }

    json_parsr = json::parse(ifstr);
    if (!parse_entries(json_parsr, categories_file_pth.parent_path()))
    {
        ifstr.close();
        goto error;
    }

    std::cout << spd::ios::set_light_green_text << "[ok]"
              << spd::ios::set_default_text << std::endl;

    ifstr.close();

    return true;

error:
    std::cout << spd::ios::set_light_red_text << "[fail]"
              << spd::ios::set_default_text << std::endl;

    return false;
}


bool program::parse_entries(json& json_parsr, const std::filesystem::path& current_source_dir)
{
    std::filesystem::path current_destination_dir;
    std::string key_str;

    for (auto it = json_parsr.begin(); it != json_parsr.end(); ++it)
    {
        key_str = it.key();
        current_destination_dir = prog_args_.destination_dir;

        if (key_str == "Icon")
        {
            if (!parse_icon(it.value(), current_source_dir, current_destination_dir))
            {
                std::cout << spd::ios::set_light_red_text
                          << "[Icon fail] "
                          << spd::ios::set_default_text;
            }
        }
        else
        {
            current_destination_dir /= spd::cast::type_cast<string_type>(key_str);
            if (!make_directory(current_destination_dir) ||
                !parse_value(it.value(), current_source_dir, current_destination_dir))
            {
                return false;
            }
        }
    }

    return true;
}


bool program::parse_value(
        json::value_type& val,
        const std::filesystem::path& current_source_dir,
        const std::filesystem::path& current_destination_dir
)
{
    std::filesystem::path shortcut_pth = current_destination_dir;

    if (val.is_boolean())
    {
        if (!val)
        {
            return true;
        }
    }
    else if (val.is_number())
    {
        shortcut_pth /= to_string(val);
        if (!make_directory(shortcut_pth))
        {
            return false;
        }
    }
    else if (val.is_string())
    {
        shortcut_pth /= spd::cast::type_cast<string_type>(std::string(val));
        if (!make_directory(shortcut_pth))
        {
            return false;
        }
    }
    else if (val.is_array())
    {
        for (auto& x : val)
        {
            if (!parse_value(x, current_source_dir, current_destination_dir))
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        return false;
    }

    shortcut_pth /= current_source_dir.filename();
    if (!make_shortcut(current_source_dir, shortcut_pth))
    {
        return false;
    }

    return true;
}


bool program::parse_icon(
        json::value_type& val,
        const std::filesystem::path& current_source_dir,
        const std::filesystem::path& current_destination_dir
)
{
    if (val.is_array())
    {
        for (auto& x : val)
        {
            if (!parse_icon(x, current_source_dir, current_destination_dir))
            {
                return false;
            }
        }

        return true;
    }

    std::filesystem::path new_destination_pth = current_destination_dir;
    if (val.is_number())
    {
        new_destination_pth /= to_string(val);
    }
    else if (val.is_string())
    {
        new_destination_pth /= spd::cast::type_cast<string_type>(std::string(val));
    }
    else
    {
        return false;
    }

    spd::sys::fsys::mkdir_recursively(new_destination_pth.c_str());
    return set_icon(current_source_dir, new_destination_pth);
}


bool program::set_icon(
        const std::filesystem::path& current_source_dir,
        const std::filesystem::path& current_destination_dir)
{
#if defined(__GNU_LIBRARY__) || defined(__CYGWIN__)
    // TODO: Implement the icon set for linux.
    return false;

#elif defined(_WIN32)
    try
    {
        std::filesystem::path source_icon_pth = current_source_dir / ".icon.ico";
        std::filesystem::path destination_icon_pth = current_destination_dir / ".icon.ico";
        spd::sys::tm::system_time source_modification_tme;
        spd::sys::tm::system_time destination_modification_tme;
        bool need_cpy = true;
        
        if (!spd::sys::fsys::file_exists(source_icon_pth.c_str()))
        {
            return false;
        }

        if (spd::sys::fsys::file_exists(destination_icon_pth.c_str()))
        {
            spd::sys::fsys::get_modification_time(source_icon_pth.c_str(),
                                                  &source_modification_tme);
            spd::sys::fsys::get_modification_time(destination_icon_pth.c_str(),
                                                  &destination_modification_tme);
            
            if (destination_modification_tme >= source_modification_tme)
            {
                inode_st_.insert(spd::sys::fsys::get_file_inode(destination_icon_pth.c_str()));
                return true;
            }
            
            SetFileAttributesW(destination_icon_pth.c_str(), 0x80);
        }
        
        std::filesystem::copy(source_icon_pth, current_destination_dir,
                              std::filesystem::copy_options::overwrite_existing);
        SetFileAttributesW(destination_icon_pth.c_str(), 0x22);
        
        inode_st_.insert(spd::sys::fsys::get_file_inode(destination_icon_pth.c_str()));
        return true;
    }
    catch (...)
    {
        return false;
    }
#endif
}


bool program::make_directory(const std::filesystem::path& directory_pth)
{
    spd::sys::fsys::mkdir(directory_pth.c_str());
    if (!spd::sys::fsys::is_directory(directory_pth.c_str()))
    {
        return false;
    }

    inode_st_.insert(spd::sys::fsys::get_file_inode(directory_pth.c_str()));
    configure_directory(directory_pth);

    return true;
}


bool program::configure_directory(const std::filesystem::path& directory_pth)
{
#if defined(__GNU_LIBRARY__) || defined(__CYGWIN__)
    // TODO: Implement the directory configuration for linux.
    return false;

#elif defined(_WIN32)
    std::filesystem::path desktop_ini_pth = directory_pth / "desktop.ini";

    if (!spd::sys::fsys::file_exists(desktop_ini_pth.c_str()))
    {
        std::ofstream ofs(desktop_ini_pth);
        if (!ofs.is_open())
        {
            return false;
        }

        ofs << "[.ShellClassInfo]\n"
               "IconResource=.icon.ico,0\n"
               "IconFile=.icon.ico\n"
               "IconIndex=0\n"
               "[ViewState]\n"
               "FolderType=Videos\n"
               "Mode=\n"
               "Vid=\n";

        ofs.close();
    }

    SetFileAttributesW(desktop_ini_pth.c_str(), 0x26);
    SetFileAttributesW(directory_pth.c_str(), 0x11);
    inode_st_.insert(spd::sys::fsys::get_file_inode(desktop_ini_pth.c_str()));

    return true;
#endif
}


bool program::make_shortcut(
        const std::filesystem::path& target_pth,
        const std::filesystem::path& shortcut_pth
)
{
    string_type shortcut_actual_pth = shortcut_pth;
    spd::sys::tm::system_time target_modification_tme;
    spd::sys::tm::system_time shortcut_modification_tme;
    std::filesystem::path target_json_pth = target_pth / prog_args_.categories_file_nme;

    shortcut_actual_pth += spd::type_casting::type_cast<string_type>(
            SPEED_SYSTEM_FILESYSTEM_SHORTCUT_EXTENSION_CSTR);
    
    if (spd::sys::fsys::file_exists(shortcut_actual_pth.c_str()))
    {
        spd::sys::fsys::get_modification_time(target_json_pth.c_str(),
                                              &target_modification_tme);
        spd::sys::fsys::get_modification_time(shortcut_actual_pth.c_str(),
                                              &shortcut_modification_tme);
        
        if (shortcut_modification_tme >= target_modification_tme)
        {
            inode_st_.insert(spd::sys::fsys::get_file_inode(shortcut_actual_pth.c_str()));
            return true;
        }
        
        spd::sys::fsys::unlink(shortcut_actual_pth.c_str());
    }

    if (!spd::sys::fsys::shortcut(target_pth.c_str(), shortcut_pth.c_str()))
    {
        return false;
    }

    inode_st_.insert(spd::sys::fsys::get_file_inode(shortcut_actual_pth.c_str()));
    return true;
}


void program::check_extra_file(const std::filesystem::path& extra_file_pth)
{
    if (!inode_st_.contains(spd::sys::fsys::get_file_inode(extra_file_pth.c_str())))
    {
        if (spd::sys::fsys::is_directory(extra_file_pth.c_str()))
        {
            std::cout << spd::ios::set_yellow_text
                      << "Found extra directory: "
                      << spd::ios::set_white_text
                      << "\""
                      << spd::cast::type_cast<std::string>(extra_file_pth.c_str())
                      << "\""
                      << spd::ios::set_default_text
                      << spd::ios::newl;

            extra_files_fnd_ = true;
        }
        else if (extra_file_pth.extension() == ".lnk" ||
                 extra_file_pth.extension() == ".ini" ||
                 extra_file_pth.extension().empty())
        {
            std::cout << spd::ios::set_yellow_text
                      << "Found extra file: "
                      << spd::ios::set_white_text
                      << "\""
                      << spd::cast::type_cast<std::string>(extra_file_pth.c_str())
                      << "\""
                      << spd::ios::set_default_text
                      << spd::ios::newl;

            extra_files_fnd_ = true;
        }
    }
}


void program::delete_extra_file(const std::filesystem::path& extra_file_pth) const
{
    if (!inode_st_.contains(spd::sys::fsys::get_file_inode(extra_file_pth.c_str())))
    {
        if (spd::sys::fsys::is_directory(extra_file_pth.c_str()))
        {
            std::cout << spd::ios::set_light_red_text
                      << "Deleting directory: "
                      << spd::ios::set_white_text
                      << "\""
                      << spd::cast::type_cast<std::string>(extra_file_pth.c_str())
                      << "\" ";

            if (spd::sys::fsys::rmdir(extra_file_pth.c_str()))
            {
                std::cout << spd::ios::set_light_green_text
                          << "[ok]"
                          << spd::ios::set_default_text
                          << spd::ios::newl;
            }
            else
            {
                std::cout << spd::ios::set_light_red_text
                          << "[fail]"
                          << spd::ios::set_default_text
                          << spd::ios::newl;
            }
        }
        else if (extra_file_pth.extension() == ".lnk" ||
                 extra_file_pth.extension() == ".ini" ||
                 extra_file_pth.extension().empty())
        {
            std::cout << spd::ios::set_light_red_text
                      << "Deleting file: "
                      << spd::ios::set_white_text
                      << "\""
                      << spd::cast::type_cast<std::string>(extra_file_pth.c_str())
                      << "\" ";

            if (spd::sys::fsys::unlink(extra_file_pth.c_str()))
            {
                std::cout << spd::ios::set_light_green_text
                          << "[ok]"
                          << spd::ios::set_default_text
                          << spd::ios::newl;
            }
            else
            {
                std::cout << spd::ios::set_light_red_text
                          << "[fail]"
                          << spd::ios::set_default_text
                          << spd::ios::newl;
            }
        }
    }
}


void program::escape_special_characters_in_regex_string(std::string& inpt)
{
    const std::string special_chars = R"([\^$.|?*+(){}])";

    for (auto it = inpt.begin(); it != inpt.end(); ++it)
    {
        if (special_chars.find(*it) != std::string::npos)
        {
            it = inpt.insert(it, '\\');
            ++it;
        }
    }
}


}
