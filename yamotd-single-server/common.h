#pragma once

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SERVER_PORT 9808
#define MAX_LINE 256

using namespace std;

string quit_command = "quit";
string msgget_command = "msgget";
string login_command = "login";
string logout_command = "logout";
string shutdown_command = "shutdown";
string msgstore_command = "msgstore";

bool startsWithNoCase(const std::string &str, const std::string &prefix)
{
    if (str.length() < prefix.length())
    {
        return false; // The string is shorter than the prefix, so it can't start with it.
    }

    for (size_t i = 0; i < prefix.length(); ++i)
    {
        if (std::tolower(str[i]) != std::tolower(prefix[i]))
        {
            return false; // Characters don't match in a case-insensitive comparison.
        }
    }

    return true;
}

bool isSameNoCase(const std::string &str1, const std::string &str2)
{
    if (str1.length() != str2.length())
    {
        return false; // If the lengths are different, the strings cannot be the same.
    }

    for (size_t i = 0; i < str1.length(); ++i)
    {
        if (std::tolower(str1[i]) != std::tolower(str2[i]))
        {
            return false; // Characters don't match in a case-insensitive comparison.
        }
    }

    return true;
}

std::string removePrefixNoCase(const std::string &str, const std::string &prefix)
{
    // Check if the string starts with the prefix (case-insensitive)
    size_t str_len = str.length();
    size_t prefix_len = prefix.length();

    if (str_len >= prefix_len)
    {
        bool match = true;
        for (size_t i = 0; i < prefix_len; ++i)
        {
            if (std::tolower(str[i]) != std::tolower(prefix[i]))
            {
                match = false;
                break;
            }
        }

        if (match)
        {
            // Return the string with the prefix removed
            return str.substr(prefix_len);
        }
    }

    // If the prefix is not found or couldn't be removed, return the original string
    return str;
}