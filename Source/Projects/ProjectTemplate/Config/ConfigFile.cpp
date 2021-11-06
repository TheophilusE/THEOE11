/**
  @author Justin Miller (carnalis.j at gmail.com)
  @author Thebluefish
  @license The MIT License (MIT)
  @copyright
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "ConfigFile.h"

#include <Urho3D/Container/Str.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/IO/File.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/IO/MemoryBuffer.h>

using namespace Urho3D;

ConfigFile::ConfigFile(Context* context, bool caseSensitive)
    : Resource(context)
    , caseSensitive_(caseSensitive)
{
}

void ConfigFile::RegisterObject(Context* context) { context->RegisterFactory<ConfigFile>(); }

bool ConfigFile::BeginLoad(Deserializer& source)
{
    unsigned dataSize(source.GetSize());
    if (!dataSize && !source.GetName().empty())
    {
        URHO3D_LOGERROR("Zero sized data in " + source.GetName());
        return false;
    }

    configMap_.push_back(ConfigSection());
    ConfigSection* configSection(&configMap_.back());
    while (!source.IsEof())
    {
        eastl::string line(source.ReadLine());

        // Parse headers.
        if (line.starts_with("[") && line.ends_with("]"))
        {
            configMap_.push_back(ConfigSection());
            configSection = &configMap_.back();
        }

        configSection->push_back(line);
    }

    return true;
}

bool ConfigFile::Save(Serializer& dest) const
{
    dest.WriteLine("# AUTO-GENERATED");

    eastl::hash_map<eastl::string, eastl::string> processedConfig;

    // Iterate over all sections, printing out the header followed by the properties.
    for (eastl::vector<ConfigSection>::const_iterator itr(configMap_.begin()); itr != configMap_.end(); ++itr)
    {
        if (itr->begin() == itr->end())
        {
            continue;
        }

        // Don't print section if there's nothing to print.
        eastl::vector<eastl::string>::const_iterator section_itr(itr->begin());
        eastl::string header(ParseHeader(*section_itr));

        // Don't print header if it's empty.
        if (!header.empty())
        {
            dest.WriteLine("");
            dest.WriteLine("[" + header + "]");
        }

        dest.WriteLine("");

        for (; section_itr != itr->end(); ++section_itr)
        {
            const eastl::string line(ParseComments(*section_itr));

            eastl::string property;
            eastl::string value;

            ParseProperty(line, property, value);

            if (processedConfig.contains(property))
            {
                continue;
            }
            processedConfig[property] = value;

            if (!property.empty() && !value.empty())
            {
                dest.WriteLine(property + "=" + value);
            }
        }

        dest.WriteLine("");
    }

    return true;
}

bool ConfigFile::Save(Serializer& dest, bool smartSave) const
{
    if (!smartSave)
    {
        return Save(dest);
    }

    eastl::hash_map<eastl::string, bool> wroteLine;
    eastl::string activeSection;

    // Iterate over all sections, printing out the header followed by the properties.
    for (eastl::vector<ConfigSection>::const_iterator itr(configMap_.begin()); itr != configMap_.end(); ++itr)
    {
        if (itr->begin() == itr->end())
        {
            continue;
        }

        for (eastl::vector<eastl::string>::const_iterator section_itr(itr->begin()); section_itr != itr->end();
             ++section_itr)
        {
            const eastl::string line(*section_itr);

            if (wroteLine.contains(activeSection + line))
            {
                continue;
            }

            wroteLine[activeSection + line] = true;

            if (line.empty())
            {
                continue;
            }

            if (section_itr == itr->begin())
            {
                dest.WriteLine("");
                dest.WriteLine("[" + line + "]");
                activeSection = line;
            }
            else
            {
                dest.WriteLine(line);
            }
        }
    }

    return true;
}

bool ConfigFile::FromString(const eastl::string& source)
{
    if (source.empty())
    {
        return false;
    }

    MemoryBuffer buffer(source.c_str(), source.length());
    return Load(buffer);
}

bool ConfigFile::Has(const eastl::string& section, const eastl::string& parameter)
{
    return !GetString(section, parameter).empty();
}

const eastl::string ConfigFile::GetString(const eastl::string& section, const eastl::string& parameter,
                                          const eastl::string& defaultValue)
{
    // Find the correct section.
    ConfigSection* configSection(nullptr);
    for (eastl::vector<ConfigSection>::iterator itr(configMap_.begin()); itr != configMap_.end(); ++itr)
    {
        if (itr->begin() == itr->end())
        {
            continue;
        }

        eastl::string& header(*(itr->begin()));
        header = ParseHeader(header);

        if (caseSensitive_)
        {
            if (section == header)
            {
                configSection = &(*itr);
            }
        }
        else
        {
            if (section.to_lower() == header.to_lower())
            {
                configSection = &(*itr);
            }
        }
    }

    // Section doesn't exist.
    if (!configSection)
    {
        return defaultValue;
    }

    for (eastl::vector<eastl::string>::const_iterator itr(configSection->begin()); itr != configSection->end(); ++itr)
    {
        eastl::string property;
        eastl::string value;
        ParseProperty(*itr, property, value);

        if (property.empty() || value.empty())
        {
            continue;
        }

        if (caseSensitive_)
        {
            if (parameter == property)
            {
                return value;
            }
        }
        else
        {
            if (parameter.to_lower() == property.to_lower())
            {
                return value;
            }
        }
    }

    return defaultValue;
}

const int ConfigFile::GetInt(const eastl::string& section, const eastl::string& parameter, const int defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToInt(property);
}

const bool ConfigFile::GetBool(const eastl::string& section, const eastl::string& parameter, const bool defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToBool(property);
}

const float ConfigFile::GetFloat(const eastl::string& section, const eastl::string& parameter, const float defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToFloat(property);
}

const Vector2 ConfigFile::GetVector2(const eastl::string& section, const eastl::string& parameter,
                                     const Vector2& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToVector2(property);
}

const Vector3 ConfigFile::GetVector3(const eastl::string& section, const eastl::string& parameter,
                                     const Vector3& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToVector3(property);
}

const Vector4 ConfigFile::GetVector4(const eastl::string& section, const eastl::string& parameter,
                                     const Vector4& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToVector4(property);
}

const Quaternion ConfigFile::GetQuaternion(const eastl::string& section, const eastl::string& parameter,
                                           const Quaternion& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToQuaternion(property);
}

const Color ConfigFile::GetColor(const eastl::string& section, const eastl::string& parameter,
                                 const Color& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToColor(property);
}

const IntRect ConfigFile::GetIntRect(const eastl::string& section, const eastl::string& parameter,
                                     const IntRect& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToIntRect(property);
}

const IntVector2 ConfigFile::GetIntVector2(const eastl::string& section, const eastl::string& parameter,
                                           const IntVector2& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToIntVector2(property);
}

const Matrix3 ConfigFile::GetMatrix3(const eastl::string& section, const eastl::string& parameter,
                                     const Matrix3& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToMatrix3(property);
}

const Matrix3x4 ConfigFile::GetMatrix3x4(const eastl::string& section, const eastl::string& parameter,
                                         const Matrix3x4& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToMatrix3x4(property);
}

const Matrix4 ConfigFile::GetMatrix4(const eastl::string& section, const eastl::string& parameter,
                                     const Matrix4& defaultValue)
{
    eastl::string property(GetString(section, parameter));

    if (property.empty())
    {
        return defaultValue;
    }

    return ToMatrix4(property);
}

void ConfigFile::Set(const eastl::string& section, const eastl::string& parameter, const eastl::string& value)
{
    // Find the correct section.
    ConfigSection* configSection(nullptr);
    for (eastl::vector<ConfigSection>::iterator itr(configMap_.begin()); itr != configMap_.end(); ++itr)
    {
        if (itr->begin() == itr->end())
        {
            continue;
        }

        eastl::string& header(*(itr->begin()));
        header = ParseHeader(header);

        if (caseSensitive_)
        {
            if (section == header)
            {
                configSection = &(*itr);
            }
        }
        else
        {
            if (section.to_lower() == header.to_lower())
            {
                configSection = &(*itr);
            }
        }
    }

    if (section.empty())
    {
        configSection = &(*configMap_.begin());
    }

    // Section doesn't exist.
    if (!configSection)
    {
        eastl::string sectionName(section);

        // Format header.
        if (ConfigFile::ParseHeader(sectionName) == sectionName)
        {
            sectionName = "[" + sectionName + "]";
        }

        // Create section.
        configMap_.push_back(ConfigSection());
        configSection = &configMap_.back();

        // Add header and blank line.
        configSection->push_back(sectionName);
        configSection->push_back("");
    }

    eastl::string* line(nullptr);
    unsigned separatorPos(0);
    for (eastl::vector<eastl::string>::iterator itr(configSection->begin()); itr != configSection->end(); ++itr)
    {
        // Find property separator.
        separatorPos = itr->find("=");
        if (separatorPos == eastl::string::npos)
        {
            separatorPos = itr->find(":");
        }

        // Not a property.
        if (separatorPos == eastl::string::npos)
        {
            continue;
        }

        eastl::string workingLine = ParseComments(*itr);

        eastl::string oldParameter(workingLine.substr(0, separatorPos).trimmed());
        eastl::string oldValue(workingLine.substr(separatorPos + 1).trimmed());

        // Not the correct parameter.
        if (caseSensitive_ ? (oldParameter == parameter) : (oldParameter.to_lower() == parameter.to_lower()))
        {
            // Replace the value.
            itr->replace(itr->find(oldValue, separatorPos), oldValue.length(), value);
            return;
        }
    }

    // Parameter doesn't exist yet.
    // Find a good place to insert the parameter, avoiding lines which are entirely comments or whitespacing.
    int index(configSection->size() - 1);
    for (int i(index); i >= 0; i--)
    {
        if (!ParseComments((*configSection)[i]).empty())
        {
            index = i + 1;
            break;
        }
    }

    // configSection->insert(index, parameter + "=" + value); // Note: Need to test this later
    configSection->insert_at(index, parameter + "=" + value);
}

// Returns header name without bracket.
const eastl::string ConfigFile::ParseHeader(eastl::string line)
{
    // Only parse comments outside of headers.
    unsigned commentPos(0);

    while (commentPos != eastl::string::npos)
    {
        // Find next comment.
        unsigned lastCommentPos(commentPos);
        unsigned commaPos(line.find("//", commentPos));
        unsigned hashPos(line.find("#", commentPos));
        commentPos = (commaPos < hashPos) ? commaPos : hashPos;

        // Header is behind a comment.
        if (line.find("[", lastCommentPos) > commentPos)
        {
            // Stop parsing this line.
            break;
        }

        // Header is before a comment.
        if (line.find("[") < commentPos)
        {
            unsigned startPos(line.find("[") + 1);
            unsigned l1(line.find("]"));
            unsigned length(l1 - startPos);
            line = line.substr(startPos, length);
            break;
        }
    }

    line = line.trimmed();

    return line;
}

const void ConfigFile::ParseProperty(eastl::string line, eastl::string& property, eastl::string& value)
{
    line = ParseComments(line);

    // Find property separator.
    unsigned separatorPos(line.find("="));
    if (separatorPos == eastl::string::npos)
    {
        separatorPos = line.find(":");
    }

    // Not a property.
    if (separatorPos == eastl::string::npos)
    {
        property = "";
        value = "";
        return;
    }

    property = line.substr(0, separatorPos).trimmed();
    value = line.substr(separatorPos + 1).trimmed();
}

const eastl::string ConfigFile::ParseComments(eastl::string line)
{
    // Normalize comment tokens.
    line.replace("//", "#");

    // Strip comments.
    unsigned commentPos(line.find("#"));
    if (commentPos != eastl::string::npos)
    {
        line = line.substr(0, commentPos);
    }

    return line;
}
