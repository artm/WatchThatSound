#ifndef LET_HPP
#define LET_HPP

/* temporarily change the value of a variable */
template<typename T>
class Let {
public:
    Let(T& var, T tempVal)
        : m_var(var), m_oldVal(var)
    {
        m_var = tempVal;
    }
    ~Let() {
        m_var = m_oldVal;
    }
private:
    T& m_var;
    T m_oldVal;
};

#endif // LET_HPP
