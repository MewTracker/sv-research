#include <QFile>
#include "Utils.h"

std::vector<uint8_t> get_resource_file(const char* filename)
{
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return std::vector<uint8_t>();
	std::vector<uint8_t> buffer;
	buffer.resize(file.size());
	QDataStream stream(&file);
	if (stream.readRawData((char*)buffer.data(), buffer.size()) != buffer.size())
		return std::vector<uint8_t>();
	return buffer;
}

bool hex_to_uint32(const QString& hex_string, uint32_t& result)
{
	return hex_to_uint32(hex_string.toStdString().c_str(), result);
}

bool hex_to_uint32(const char* hex_string, uint32_t& result)
{
	char* end_ptr = nullptr;
	errno = 0;
	result = strtoul(hex_string, &end_ptr, 16);
	if ((size_t)(end_ptr - hex_string) != strlen(hex_string) || errno == ERANGE)
		return false;
	return true;
}

QString format_uint32(uint32_t value)
{
	return QString("%1").arg(value, 8, 16, QLatin1Char('0')).toUpper();
}

void select_option(QComboBox *combo, uint32_t value)
{
	for (int i = 0; i < combo->count(); ++i)
	{
		if (combo->itemData(i).toUInt() == value)
		{
			combo->setCurrentIndex(i);
			break;
		}
	}
}
