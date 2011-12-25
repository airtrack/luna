#ifndef STL_UTILITY_H
#define STL_UTILITY_H

template<typename Container>
inline void erase_elements(Container& c)
{
    for (auto it = c.begin(); it != c.end(); ++it)
        delete (*it);
    c.clear();
}

#endif // STL_UTILITY_H
