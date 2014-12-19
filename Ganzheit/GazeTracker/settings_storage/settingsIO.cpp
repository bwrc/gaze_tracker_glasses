#include <iostream>
#include <sstream>
#include <tinyxml.h>

#include "settingsIO.h"

SettingsIO::SettingsIO()
{
	this->fileAvailable = false;
}

SettingsIO::SettingsIO(const std::string& sourceFile)
{
	this->fileAvailable = false;
	this->open(sourceFile);
	this->filename = sourceFile;
}

bool SettingsIO::open(const std::string& sourceFile)
{
	document = TiXmlDocument(sourceFile.c_str());
	fileAvailable = document.LoadFile();
	return fileAvailable;
}

bool SettingsIO::get(const std::string& filename, const std::string& parameter, std::string& value)
{
	// Is the file available?
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		return false;
	}

	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(!fileElement) {
		return false;
	}

	for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
		const char* name_chr = fileElement->Attribute("filename");
		if(!name_chr) {
			continue;
		}

		if(std::string(name_chr) == filename) {
			break;
		}
	}

	if(!fileElement) {
		return false;
	}

	// Is there a field for the parameter?
	TiXmlElement *parameterElement = fileElement->FirstChildElement(parameter.c_str());
	if(!parameterElement) {
		return false;
	}

	// Does the field have a value?
	const char* valueData = parameterElement->Attribute("value");
	if(!valueData) {
		return false;
	}

	value = std::string(valueData);

	return true;
}

bool SettingsIO::get(const std::string& filename, const std::string& parameter, int& value)
{

	std::string s_value;

	if(!get(filename,parameter, s_value)) {
		return false;
	}

	if((std::stringstream(s_value) >> value).fail()) {
		return false;
	}

	return true;
}

bool SettingsIO::get(const std::string& filename, const std::string& parameter, unsigned int& value)
{

	std::string s_value;

	if(!get(filename,parameter, s_value)) {
		return false;
	}

	if((std::stringstream(s_value) >> value).fail()) {
		return false;
	}

	return true;
}

bool SettingsIO::get(const std::string& filename, const std::string& parameter, double& value)
{

	std::string s_value;

	if(!get(filename,parameter, s_value)) {
		return false;
	}

	if((std::stringstream(s_value) >> value).fail()) {
		return false;
	}

	return true;
}


bool SettingsIO::set(const std::string& filename, const std::string& parameter, const std::string& value)
{
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		rootElement = (TiXmlElement *)document.InsertEndChild(TiXmlElement("document"));
	}


	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(fileElement) {
		for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
			const char* name_chr = fileElement->Attribute("filename");
			if(!name_chr) {
				continue;
			}

			if(std::string(name_chr) == filename) {
				break;
			}
		}
	}

	// Does the given file have any settings?
	if(!fileElement) {
		// No. Add an element for the file
		fileElement = (TiXmlElement *)document.RootElement()->InsertEndChild(TiXmlElement("settings"));
		fileElement->SetAttribute("filename", filename.c_str());
	}

	// Does the given parameter already have some value?
	TiXmlElement *parameterElement = fileElement->FirstChildElement(parameter.c_str());
	if(!parameterElement) {
		// No. Create a parameter element
		parameterElement = (TiXmlElement *)fileElement->InsertEndChild(TiXmlElement(parameter.c_str()));
	}
	
	// Set value for the parameter
	parameterElement->SetAttribute("value", value.c_str());

	return true;
}

bool SettingsIO::set(const std::string& filename, const std::string& parameter, const unsigned int value)
{
	std::stringstream value_s;
	value_s << value;
	return set(filename, parameter, value_s.str());
}

bool SettingsIO::set(const std::string& filename, const std::string& parameter, const int value)
{
	std::stringstream value_s;
	value_s << value;
	return set(filename, parameter, value_s.str());
}

bool SettingsIO::set(const std::string& filename, const std::string& parameter, const double value)
{
	std::stringstream value_s;
	value_s << value;
	return set(filename, parameter, value_s.str());
}

bool SettingsIO::getFields(const std::string& filename, const std::string& fieldName, std::list<std::string>& fields)
{

	// Is the file available?
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		return false;
	}

	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(!fileElement) {
		return false;
	}

	for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
		const char* name_chr = fileElement->Attribute("filename");
		if(!name_chr) {
			continue;
		}

		if(std::string(name_chr) == filename) {
			break;
		}
	}

	if(!fileElement) {
		return false;
	}

	// Is there a field for the parameter?
	TiXmlElement *parameterElement = fileElement->FirstChildElement(fieldName.c_str());
	if(!parameterElement) {
		return false;
	}

	fields.empty();

	for( ; parameterElement; parameterElement = parameterElement->NextSiblingElement() ) {
		const char* name_chr = parameterElement->Attribute("value");

		if(!name_chr) {
			continue;
		}

		fields.push_back(std::string(name_chr));
	}

	return true;
}

void SettingsIO::setFields(const std::string& filename, const std::string& fieldName, std::list<std::string>& fields)
{
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		rootElement = (TiXmlElement *)document.InsertEndChild(TiXmlElement("document"));
	}


	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(fileElement) {
		for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
			const char* name_chr = fileElement->Attribute("filename");
			if(!name_chr) {
				continue;
			}

			if(std::string(name_chr) == filename) {
				break;
			}
		}
	}

	// Does the given file have any settings?
	if(!fileElement) {
		// No. Add an element for the file
		fileElement = (TiXmlElement *)document.RootElement()->InsertEndChild(TiXmlElement("settings"));
		fileElement->SetAttribute("filename", filename.c_str());
	}

	// Remove all nodes of this file.
	fileElement->Clear();

	std::list<std::string>::iterator field = fields.begin();

	for(; field != fields.end(); field++) {

		// Set value for the parameter
		TiXmlElement *parameterElement = (TiXmlElement *)fileElement->InsertEndChild(TiXmlElement(fieldName.c_str()));
		parameterElement->SetAttribute("value", field->c_str());
	}
}

bool SettingsIO::setVector(const std::string& filename, const std::string& vectorName, const std::vector<double>& vect)
{
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		rootElement = (TiXmlElement *)document.InsertEndChild(TiXmlElement("document"));
	}


	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(fileElement) {
		for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
			const char* name_chr = fileElement->Attribute("filename");
			if(!name_chr) {
				continue;
			}

			if(std::string(name_chr) == filename) {
				break;
			}
		}
	}

	// Does the given file have any settings?
	if(!fileElement) {
		// No. Add an element for the file
		fileElement = (TiXmlElement *)document.RootElement()->InsertEndChild(TiXmlElement("settings"));
		fileElement->SetAttribute("filename", filename.c_str());
	}

	// Does the given parameter already have some value?
	TiXmlElement *vectorElement = fileElement->FirstChildElement(vectorName.c_str());
	if(!vectorElement) {
		// No. Create a vector element
		vectorElement = (TiXmlElement *)fileElement->InsertEndChild(TiXmlElement(vectorName.c_str()));
	}

	std::stringstream valueStream;
	for(std::vector<double>::const_iterator it; it != vect.end(); it++) {
		valueStream << *it;

		// The elements are separated with spaces
		if(it+1 != vect.end()) {
			valueStream << " ";
		}
	}

	std::string value = valueStream.str();

	// Set value for the parameter
	vectorElement->SetAttribute("value", value.c_str());

	return true;
}

bool SettingsIO::getVector(const std::string& filename, const std::string& vectorName, std::vector<double>& vect)
{
	// Is the file available?
	TiXmlElement *rootElement = document.RootElement();
	if(!rootElement) {
		return false;
	}

	// Is there any setting for the given file?
	TiXmlElement *fileElement = rootElement->FirstChildElement("settings");
	if(!fileElement) {
		return false;
	}

	for( ; fileElement; fileElement = fileElement->NextSiblingElement() ) {
		const char* name_chr = fileElement->Attribute("filename");
		if(!name_chr) {
			continue;
		}

		if(std::string(name_chr) == filename) {
			break;
		}
	}

	if(!fileElement) {
		return false;
	}

	// Is there a field for the parameter?
	TiXmlElement *vectorElement = fileElement->FirstChildElement(vectorName.c_str());
	if(!vectorElement) {
		return false;
	}

	// Does the field have a value?
	const char* valueData = vectorElement->Attribute("value");
	if(!valueData) {
		return false;
	}

	vect.clear();

	// Extract the elements from the file
	std::stringstream ss(valueData);

	double value;

	// Go through each word in the line
	while (ss >> value) {
		vect.push_back(value);
	}

	return true;
}

bool SettingsIO::save(const std::string& targetFile)
{
	return document.SaveFile(targetFile.c_str());
}

bool SettingsIO::save(void)
{
	if(this->filename.empty()) {
		return false;
	}

	return document.SaveFile(filename.c_str());
}

