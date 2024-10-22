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
 * @file        classifier/program.hpp
 * @brief       program class header.
 * @author      Killian Valverde
 * @date        2024/10/15
 */
 
#ifndef CLASSIFIER_PROGRAM_HPP
#define CLASSIFIER_PROGRAM_HPP

#include <speed/speed.hpp>

#include "exception.hpp"
#include "json.hpp"
#include "program_args.hpp"


/**
 * @brief       Contians all classifier resources.
 */
namespace classifier {


class program
{
public:
    using char_type = std::filesystem::path::value_type;

    using string_type = std::basic_string<char_type>;

    /**
     * @brief       Constructor with parameters.
     * @param       prog_args : The program arguments.
     */
    explicit program(program_args&& prog_args);
    
    /**
     * @brief       Execute the program.
     * @return      The value that represents if the program succeed.
     */
    int execute();

private:
    bool parse_categories_file(const std::filesystem::path& categories_file_pth);

    bool parse_entries(json& json_parsr, const std::filesystem::path& current_source_dir);

    bool parse_value(
            json::value_type& val,
            const std::filesystem::path& current_source_dir,
            const std::filesystem::path& current_destination_dir
    );

    bool parse_icon(
            json::value_type& val,
            const std::filesystem::path& current_source_dir,
            const std::filesystem::path& current_destination_dir
    );

    bool set_icon(
            const std::filesystem::path& current_source_dir,
            const std::filesystem::path& current_destination_dir
    );

    bool make_directory(const std::filesystem::path& directory_pth);

    bool configure_directory(const std::filesystem::path& directory_pth);

    bool make_shortcut(
            const std::filesystem::path& target_pth,
            const std::filesystem::path& shortcut_pth
    );

    void check_extra_file(const std::filesystem::path& extra_file_pth);

    void delete_extra_file(const std::filesystem::path& extra_file_pth) const;

    void escape_special_characters_in_regex_string(std::string& inpt);

private:
    /** The program arguments. */
    program_args prog_args_;

    std::set<std::uint64_t> inode_st_;

    bool extra_files_fnd_;
};


}


#endif
