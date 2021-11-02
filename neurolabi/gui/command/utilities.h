#ifndef COMMAND_UTILITIES_H
#define COMMAND_UTILITIES_H

#include <vector>
#include <iostream>

namespace neutu {

std::vector<uint64_t> ImportBodies(std::istream &stream);
std::vector<uint64_t> ImportBodies(const std::string &filePath);

/*!
 * \brief Import bodies from csv
 *
 * \param column the column position (0-indexed) of body ids
 * \param hasHead the file has head or not
 * \return a list of body ids
 */
std::vector<uint64_t> ImportBodiesFromCsv(
    std::istream &stream, size_t column, bool hasHead);
std::vector<uint64_t> ImportBodiesFromCsv(
    const std::string &filePath, size_t column, bool hasHead);

}

#endif // UTILITIES_H
