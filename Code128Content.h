#pragma once
#include "Code128Code.h"
#include <string>
#include <vector>

namespace GenCode128 {

/// <summary>
/// Represent the set of code values to be output into barcode form
/// </summary>
class Code128Content
{
public:
    /// <summary>
    /// Create content based on a string of ASCII data
    /// </summary>
    /// <param name="asciiData">the string that should be represented</param>
    Code128Content(const std::string& asciiData)
    {
        codes = StringToCode128(asciiData);
    }

    /// <summary>
    /// Provides the Code128 code values representing the object's string
    /// </summary>
    const std::vector<int>& GetCodes()
    {
        return codes;
    }
private:
    std::vector<int> codes;

    /// <summary>
    /// Transform the string into integers representing the Code128 codes
    /// necessary to represent it
    /// </summary>
    /// <param name="asciiData">String to be encoded</param>
    /// <returns>Code128 representation</returns>
    std::vector<int> StringToCode128(const std::string& asciiData)
    {
        // decide which codeset to start with
        auto csa1 = asciiData.size() > 0
            ? Code128Code::CodesetAllowedForChar(asciiData[0])
            : Code128Code::CodeSetAllowed::CodeAorB;
        auto csa2 = asciiData.size() > 1
            ? Code128Code::CodesetAllowedForChar(asciiData[1])
            : Code128Code::CodeSetAllowed::CodeAorB;

        auto currentCodeSet = GetBestStartSet(csa1, csa2);

        // set up the beginning of the barcode
        // assume no codeset changes, account for start, checksum, and stop
        std::vector<int> results;
        results.push_back(Code128Code::StartCodeForCodeSet(currentCodeSet));

        // add the codes for each character in the string
        for (size_t i = 0; i < asciiData.size(); i++)
        {
            int thischar = asciiData[i];
            int nextchar = asciiData.size() > i + 1 ? asciiData[i + 1] : -1;

            auto result = Code128Code::CodesForChar(thischar, nextchar, currentCodeSet);
            results.push_back(result.codes[0]);

            if (result.length > 1)
            {
                results.push_back(result.codes[1]);
            }
        }

        // calculate the check digit

        int checksum = results[0];
        for (size_t i = 1; i < results.size(); i++)
        {
            checksum += i * results[i];
        }

        results.push_back(checksum % 103);
        results.push_back(Code128Code::StopCode());
        return results;
    }

    /// <summary>
    /// Determines the best starting code set based on the the first two 
    /// characters of the string to be encoded
    /// </summary>
    /// <param name="csa1">First character of input string</param>
    /// <param name="csa2">Second character of input string</param>
    /// <returns>The codeset determined to be best to start with</returns>
    CodeSet GetBestStartSet(Code128Code::CodeSetAllowed csa1, Code128Code::CodeSetAllowed csa2)
    {
        auto vote = 0;
        vote += csa1 == Code128Code::CodeSetAllowed::CodeA ? 1 : 0;
        vote += csa1 == Code128Code::CodeSetAllowed::CodeB ? -1 : 0;
        vote += csa2 == Code128Code::CodeSetAllowed::CodeA ? 1 : 0;
        vote += csa2 == Code128Code::CodeSetAllowed::CodeB ? -1 : 0;

        return vote > 0 ? CodeSet::CodeA : CodeSet::CodeB; // ties go to codeB due to my own prejudices
    }
};

}
