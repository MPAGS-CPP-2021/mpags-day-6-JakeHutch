#ifndef MPAGSCIPHER_CUSTOMEXCEPTIONS_HPP
#define MPAGSCIPHER_CUSTOMEXCEPTIONS_HPP

#include <string>
#include <stdexcept>

/**
 * \file CustomExceptions.hpp
 * \brief Contains the declarations of the custom exception classes used throughout the code
 */

/**
 * \class MissingArguement
 * \brief Thrown whenever a required command line arguement cannot be found
 */ 
class MissingArgument : public std::invalid_argument {
    public:
        /// constructor delegated to std::invalid_argument base class
        MissingArgument( const std::string& msg ) : std::invalid_argument(msg)
    {
    }
};

/**
 * \class UnknownArguement
 * \brief Thrown whenever a command line arguement is not recognised
 */ 
class UnknownArgument : public std::invalid_argument {
    public:
        /// constructor delegated to std::invalid_argument base class
        UnknownArgument( const std::string& msg ) : std::invalid_argument(msg)
    {
    }
};

/**
 * \class InvalidKey
 * \brief Thrown whenever a problem is encountered with setting the cipher key
 */ 
class InvalidKey : public std::invalid_argument {
    public:
        /// constructor delegated to std::invalid_argument base class
        InvalidKey( const std::string& msg ) : std::invalid_argument(msg)
    {
    }
};

#endif