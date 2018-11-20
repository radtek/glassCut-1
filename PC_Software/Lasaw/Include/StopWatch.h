// StopWatch.h
#pragma once

class CStopWatch
{
    // Construction
public:
	CStopWatch(BYTE averageCount=10);
    ~CStopWatch();

    // Implementation
public:
	void Start();
	void Stop();
	void Reset(bool bOnlySts=false);
	double GetAverageTime();
	double GetTimeSpan();

    int GetAverageCount();
    void SetAverageCount(int averageCount);

protected:
	double          m_Average;
	double          m_dTimeSpan;
	int             m_AverageCount;
    int*            m_pAverageArray;
    long            m_AverageSum;
    int             m_AverageArrayIndex;
	bool            m_bIsStart;
	bool            m_bIsReset;
	LARGE_INTEGER	m_liFreq;
    LARGE_INTEGER	m_liStart;
    LARGE_INTEGER	m_liStop;
	LARGE_INTEGER	m_liLast;
};
