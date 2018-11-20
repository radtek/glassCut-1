#include "stdafx.h"
#include "StopWatch.h"


// Default Constructor

CStopWatch::CStopWatch(BYTE averageCount)
{
    QueryPerformanceFrequency(&m_liFreq);
    m_AverageCount = averageCount>0?averageCount:10;

    m_pAverageArray = new int[m_AverageCount];
    m_bIsReset = true;
	m_bIsStart = false;
    m_Average = 0.0;
    m_AverageArrayIndex = 0;
	QueryPerformanceCounter(&m_liLast);
	m_liStart = m_liLast;
}

CStopWatch::~CStopWatch()
{
    if (m_pAverageArray)
        delete [] m_pAverageArray;
    m_pAverageArray = NULL;
}

void CStopWatch::Start(void)
{
    QueryPerformanceCounter(&m_liStart);
	m_bIsStart = true;
}

void CStopWatch::Stop(void)
{
	if (!m_bIsStart)
		return;
	m_bIsStart = false;
	QueryPerformanceCounter(&m_liStop);

    // Calculate the time difference between Start and Stop
    int iTimeSpanUs = (int)((m_liStop.QuadPart - m_liStart.QuadPart) * 1000000 / m_liFreq.QuadPart);
    if (m_bIsReset)
    {
        m_bIsReset = false;
        m_AverageSum = 0;
        for (int i = 0; i < m_AverageCount; i++)
        {
            m_pAverageArray[i] = iTimeSpanUs;
            m_AverageSum += iTimeSpanUs;
        }
    }

    m_AverageSum += iTimeSpanUs - m_pAverageArray[m_AverageArrayIndex];
    m_pAverageArray[m_AverageArrayIndex] = iTimeSpanUs;

    m_AverageArrayIndex++;
    if (m_AverageArrayIndex >= m_AverageCount)
        m_AverageArrayIndex = 0;

    m_Average = (double)m_AverageSum / (double)m_AverageCount;
}

double CStopWatch::GetTimeSpan()
{
	LARGE_INTEGER	m_last = m_liLast;
	QueryPerformanceCounter(&m_liLast);
	double dTimeSpanUs = (m_liLast.QuadPart - m_last.QuadPart) * 1000000 / m_liFreq.QuadPart;
	return dTimeSpanUs;
}

void CStopWatch::Reset(bool bOnlySts)
{
	if (bOnlySts)
	{
		m_bIsStart = false;
		return;
	}
    for (int i = 0; i < m_AverageCount; i++)
    {
        m_pAverageArray[i] = 0;
    }
    m_AverageSum = 0;
	m_Average = 0.0;
	m_bIsReset = true;
}

double CStopWatch::GetAverageTime(void)
{
    return m_Average;
}

int CStopWatch::GetAverageCount(void)
{
    return m_AverageCount;
}

void CStopWatch::SetAverageCount(int averageCount)
{
    // Any changes?
    if (m_AverageCount != averageCount)
    {
        m_AverageCount = averageCount;
        delete [] m_pAverageArray;
        m_pAverageArray = new int[m_AverageCount];
    }
}
