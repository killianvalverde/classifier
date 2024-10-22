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
 * @file        classifier/program_args.hpp
 * @brief       program_args struct header.
 * @author      Killian Valverde
 * @date        2024/10/15
 */
 
#ifndef CLASSIFIER_PROGRAM_ARGS_HPP
#define CLASSIFIER_PROGRAM_ARGS_HPP

#include <speed/speed.hpp>


namespace classifier {


/**
 * @brief       All the arguments that are forwarded to the program class.
 */
struct program_args
{
    spd::fsys::rx_directory_path source_dir;
    spd::fsys::output_directory_path destination_dir;
    std::string categories_file_nme = ".categories.json";
};


}


#endif
