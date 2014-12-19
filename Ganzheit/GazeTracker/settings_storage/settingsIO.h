#ifndef SETTINGS_IO_H
#define SETTINGS_IO_H

#include <iostream>
#include <tinyxml.h>
#include <list>
#include <vector>


class SettingsIO {
	public:
		SettingsIO();
		SettingsIO(const std::string& sourceFile);
		bool isOpen(void) { return fileAvailable; }
		bool open(const std::string& sourceFile);

		bool get(const std::string& filename, const std::string& parameter, unsigned int& value);
		bool get(const std::string& filename, const std::string& parameter, int& value);
		bool get(const std::string& filename, const std::string& parameter, double& value);
		bool get(const std::string& filename, const std::string& parameter, std::string& value);
		bool getFields(const std::string& filename, const std::string& fieldName, std::list<std::string>& fields);
		bool getVector(const std::string& filename, const std::string& vectorName, std::vector<double>& vect);

		bool set(const std::string& filename, const std::string& parameter, const unsigned int value);
		bool set(const std::string& filename, const std::string& parameter, const int value);
		bool set(const std::string& filename, const std::string& parameter, const double value);
		bool set(const std::string& filename, const std::string& parameter, const std::string& value);
		void setFields(const std::string& filename, const std::string& fieldName, std::list<std::string>& fields);
		bool setVector(const std::string& filename, const std::string& vectorName, const std::vector<double>& vect);

		bool save(const std::string& targetFile);
		bool save(void);

	private:
		std::string filename;
		TiXmlDocument document;
		bool fileAvailable;
};

#endif

