#ifndef AVERAGER_H
#define AVERAGER_H

#include <vector>


template <class T>
class Averager {

public:

    Averager(size_t nCount) : m_vecValues(0), m_nCount(nCount), m_nIdx(0), m_dSum(0.0) {}


    void add(T tVal) {

        if(m_vecValues.size() < m_nCount) {
            m_vecValues.push_back(tVal);
        }
        else {

            m_dSum -= (double)m_vecValues[m_nIdx];
            m_vecValues[m_nIdx] = tVal;
            m_nIdx = (m_nIdx + 1) % m_nCount;

        }

        m_dSum += (double)tVal;

    }


    double average() {
        double dDen = std::min(m_nCount, m_vecValues.size());
        dDen = std::max(1.0, dDen);
        return m_dSum / dDen;
    }

private:

    std::vector<T> m_vecValues;
    size_t m_nCount; // 
    int m_nIdx; // where to put next value
    double m_dSum;

};


#endif

