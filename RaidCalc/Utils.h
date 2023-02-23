#pragma once

#include <QString>
#include <QComboBox>
#include <cstdint>
#include <vector>

std::vector<uint8_t> get_resource_file(const char* filename);
bool hex_to_uint32(const QString& hex_string, uint32_t& result);
bool hex_to_uint32(const char* hex_string, uint32_t& result);
QString format_uint32(uint32_t value);
void select_option(QComboBox *combo, uint32_t value);
