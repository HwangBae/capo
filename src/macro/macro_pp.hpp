/*
    The Capo Library
    Code covered by the MIT License

    Author: mutouyun (http://darkc.at)
*/

#ifndef CAPO_MACRO_PP_HPP___
#define CAPO_MACRO_PP_HPP___

////////////////////////////////////////////////////////////////

/*
    Get a string of macro
*/

#define CAPO_PP_SOL(...)                  #__VA_ARGS__
#define CAPO_PP_STR_(...)                 CAPO_PP_SOL(__VA_ARGS__)
#define CAPO_PP_STR(...)                  CAPO_PP_STR_(__VA_ARGS__)

/*
    Connect two args together
*/

#define CAPO_PP_CAT(X, ...)               X##__VA_ARGS__
#define CAPO_PP_JOIN_(X, ...)             CAPO_PP_CAT(X, __VA_ARGS__)
#define CAPO_PP_JOIN(X, ...)              CAPO_PP_JOIN_(X, __VA_ARGS__)

////////////////////////////////////////////////////////////////

#endif // CAPO_MACRO_PP_HPP___
