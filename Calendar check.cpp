#include <cstdio>
#include <vector>
#include <algorithm>

const int k_maxDays = 1000000;

// This is an application to check the error of a calendar system to find
// the best solution to translate planetary rotation and translation into
// a calendar of natural numbers

struct Node
{
    int m_days;
    int m_base;
    float m_correction;

    Node()
    {
        m_days = 0;
        m_base = 10;
        m_correction = 0.0f;
    }

    void Set( int days, float correction )
    {
        m_days = days;
        m_correction = correction;

        while ( days > m_base )
        {
            m_base *= 10;
        }
    }

    void operator=( Node &other )
    {
        m_days = other.m_days;
        m_base = other.m_base;
        m_correction = other.m_correction;
    }
};

struct CalendarInfo
{
    float m_errorPercentage;
    std::pair<int, float> m_minDiff;
    std::pair<int, float> m_maxDiff;
    float m_meanDiff;
    float m_meanBelow1;
    float m_meanAbove1;
    int m_below1;
    int m_above1;

    int m_nCorrectionsEachNYears;
    int m_nCorrectionsYearsEndIn;
    Node m_correctionsEachNYears[ 10 ];
    Node m_correctionsYearsEndIn[ 10 ];

    void ShowCalendarInfo()
    {
        printf( "\n>>Calendar Information:\n" );
        printf( "- There is a %.2f%% of years from a total of %d years that miss more than 1 day from natural translation cycle.\n", m_errorPercentage * 100.0f, k_maxDays );
        printf( "- The min diff is the year #%d with %.4f days of difference\n", m_minDiff.first, m_minDiff.second );
        printf( "- The max diff is the year #%d with %.4f days of difference\n", m_maxDiff.first, m_maxDiff.second);
        printf( "- The average diff is %.4f, with %d years below the negative limit, and %d above the positive limit\n", m_meanDiff, m_below1, m_above1 );
        printf( "- There average diff below the negative limit of difference is %.4f, and above the positive limit is %.4f\n", m_meanBelow1, m_meanAbove1 );

        printf( "\n- %s\n", (m_nCorrectionsEachNYears == 0 ? "No correction each n years" : "Corrections each n years:") );
        for ( int i = 0; i < m_nCorrectionsEachNYears; ++i )
        {
            printf( "\t> Each %d, %s %.2f day%s\n", m_correctionsEachNYears[ i ].m_days, ( m_correctionsEachNYears[ i ].m_correction < 0.0f ? "add" : "substract" ), m_correctionsEachNYears[ i ].m_correction, (std::abs( m_correctionsEachNYears[ i ].m_correction )==1.0f ? "" : "s") );
        }
        printf( "\n- %s\n", ( m_nCorrectionsYearsEndIn == 0 ? "No correction end in n" : "Corrections years end in n:" ) );
        for ( int i = 0; i < m_nCorrectionsYearsEndIn; ++i )
        {
            printf( "\t> Every year end in %d, %s 1 day\n", m_correctionsYearsEndIn[ i ].m_days, ( m_correctionsYearsEndIn[ i ].m_correction < 0.0f ? "add" : "substract" ) );
        }
        printf( ">>\n" );
    }

    bool operator<( CalendarInfo &other )
    {
        const float lambda = 0.01f;
        return ( other.m_errorPercentage - m_errorPercentage > lambda )
            || ( other.m_errorPercentage - m_errorPercentage <= lambda && m_maxDiff < other.m_maxDiff );
    }

    void operator=( CalendarInfo &other )
    {
        m_errorPercentage = other.m_errorPercentage;
        m_minDiff = other.m_minDiff;
        m_maxDiff = other.m_maxDiff;
        m_meanDiff = other.m_meanDiff;
        m_meanBelow1 = other.m_meanBelow1;
        m_meanAbove1 = other.m_meanAbove1;
        m_below1 = other.m_below1;
        m_above1 = other.m_above1;

        m_nCorrectionsEachNYears = other.m_nCorrectionsEachNYears;
        for ( int i = 0; i < m_nCorrectionsEachNYears; ++i )
        {
            m_correctionsEachNYears[ i ] = other.m_correctionsEachNYears[ i ];
        }
        m_nCorrectionsYearsEndIn = other.m_nCorrectionsYearsEndIn;
        for ( int i = 0; i < m_nCorrectionsYearsEndIn; ++i )
        {
            m_correctionsYearsEndIn[ i ] = other.m_correctionsYearsEndIn[ i ];
        }
    }

    bool GoodEnough(float threshold)
    {
        return m_errorPercentage < threshold;
    }
};

struct Calendar
{
    float m_days[k_maxDays + 1];
    float m_daysPerYearError;
    float m_daysAsError;

    int m_nCorrectionsEachNYears;
    int m_nCorrectionsYearsEndIn;
    Node m_correctionsEachNYears[ 10 ];
    Node m_correctionsYearsEndIn[ 10 ];

    Calendar(float daysPerYearError, float daysAsErrors)
    {
        m_daysPerYearError = daysPerYearError;
        m_daysAsError = daysAsErrors;
        m_days[ 0 ] = 0.0f;
        float acc = 0.0f;
        for ( int i = 1; i <= k_maxDays; ++i )
        {
            m_days[ i ] = acc + m_daysPerYearError;
            acc += m_daysPerYearError;
        }
        m_nCorrectionsEachNYears = 0;
        m_nCorrectionsYearsEndIn = 0;
    }
    
    void AddCorrectionEachNYears( Node &node )
    {
        if ( m_nCorrectionsEachNYears < 10 )
        {
            m_correctionsEachNYears[ m_nCorrectionsEachNYears++ ] = node;
        }
    }

    void AddCorrectionYearsEndIn( Node &node )
    {
        if ( m_nCorrectionsYearsEndIn < 10 )
        {
            m_correctionsYearsEndIn[ m_nCorrectionsYearsEndIn++ ] = node;
        }
    }
    
    void RemoveCorrectionEachNYears()
    {
        m_nCorrectionsEachNYears = std::max( m_nCorrectionsEachNYears - 1, 0 );
    }

    void RemoveCorrectionYearsEndIn()
    {
        m_nCorrectionsYearsEndIn = std::max( m_nCorrectionsYearsEndIn - 1, 0 );
    }

    float ApplyCorrections()
    {
        float errorPercentage = 0.0f;
        
        int errorDays = 0;
        float acc = 0.0f;
        for ( int i = 1; i <= k_maxDays; ++i )
        {
            m_days[ i ] = acc + m_daysPerYearError;
            
            for ( int j = 0; j < m_nCorrectionsEachNYears; ++j )
            {
                if ( i % m_correctionsEachNYears[j].m_days == 0 )
                {
                    m_days[ i ] += m_correctionsEachNYears[ j ].m_correction;
                }
            }

            for ( int j = 0; j < m_nCorrectionsYearsEndIn; ++j )
            {
                if ( i % m_correctionsYearsEndIn[j].m_base == m_correctionsYearsEndIn[ j ].m_days )
                {
                    m_days[ i ] += m_correctionsEachNYears[ j ].m_correction;
                }
            }

            acc = m_days[ i ];

            if ( m_days[ i ] < -1.0f || m_days[ i ] > 1.0f )
            {
                ++errorDays;
            }
        }
        errorPercentage = ( float ) errorDays / k_maxDays;

        return errorPercentage;
    }

    void GetCurrentCalendarInfo(CalendarInfo &calendarInfo)
    {
        int errorDays = 0;
        std::pair<int, float> minDiff(0, 0.0f);
        std::pair<int, float> maxDiff(0, 0.0f);
        float meanDiff = 0.0f;
        float meanBelow1 = 0.0f;
        float meanAbove1 = 0.0f;
        int below1 = 0;
        int above1 = 0;
        for ( int i = 1; i <= k_maxDays; ++i )
        {
            float d = m_days[ i ];
            meanDiff += d;
            if ( d <= -m_daysAsError || d >= m_daysAsError )
            {
                ++errorDays;
                if ( d <= -1.0f )
                {
                    meanBelow1 += d;
                    ++below1;
                }
                else
                {
                    meanAbove1 += d;
                    ++above1;
                }
            }

            if ( d < minDiff.second )
            {
                minDiff.first = i;
                minDiff.second = d;
            }
            else if ( d > maxDiff.second )
            {
                maxDiff.first = i;
                maxDiff.second = d;
            }
        }
        
        calendarInfo.m_above1 = above1;
        calendarInfo.m_below1 = below1;
        calendarInfo.m_minDiff = minDiff;
        calendarInfo.m_maxDiff = maxDiff;
        calendarInfo.m_meanDiff = meanDiff / k_maxDays;
        calendarInfo.m_meanAbove1 = above1 == 0 ? 0.0f : meanAbove1 / above1;
        calendarInfo.m_meanBelow1 = below1 == 0 ? 0.0f : meanBelow1 / below1;
        calendarInfo.m_errorPercentage = ( float ) errorDays / k_maxDays;

        calendarInfo.m_nCorrectionsEachNYears = m_nCorrectionsEachNYears;
        for ( int i = 0; i < m_nCorrectionsEachNYears; ++i )
        {
            calendarInfo.m_correctionsEachNYears[ i ] = m_correctionsEachNYears[ i ];
        }
        calendarInfo.m_nCorrectionsYearsEndIn = m_nCorrectionsYearsEndIn;
        for ( int i = 0; i < m_nCorrectionsYearsEndIn; ++i )
        {
            calendarInfo.m_correctionsYearsEndIn[ i ] = m_correctionsYearsEndIn[ i ];
        }
    }

    void ShowCalendar()
    {
        printf( "\n>> Calendar:\n\n" );
        for ( int i = 1; i <= k_maxDays; i += 4 )
        {
            printf( "#%d: %.4f\t#%d: %.4f\t#%d: %.4f\t#%d: %.4f\n", i, m_days[ i ], i + 1, m_days[ i + 1 ], i + 2, m_days[ i + 2 ], i + 3, m_days[ i + 3 ] );
        }
        printf( "\n>>\n" );
    }
};

class CalendarSolver
{
private:
    static const int k_maxConditionYear = 400;
    static const int k_maxSolutions = 10;
    const int k_maxIterations = 2;

    float m_errorThreshold;
    float m_errorPerYear;
    float m_daysAsError;
    CalendarInfo m_solutions[k_maxSolutions];
    int m_currentSolutions;
    bool m_conditionsEachNYears[ k_maxConditionYear + 1 ];
    bool m_conditionsYearsEndIn[ k_maxConditionYear + 1 ];

public:
    CalendarSolver( float errorPerYear, float daysAsError ) { Init( errorPerYear, daysAsError ); }
    ~CalendarSolver() {}

    void Solver()
    {
        Calendar calendar( m_errorPerYear, m_daysAsError );
        SolverIteration( calendar, 0, 1.0f );
    }

    void ShowSolutions()
    {
        printf( "\n>> Solutions found: %d\n\n", m_currentSolutions );
        for ( int i = 0; i < m_currentSolutions; ++i )
        {
            m_solutions[ i ].ShowCalendarInfo();
        }
    }

private:

    void Init( float errorPerYear, float daysAsError )
    {
        m_errorThreshold = 0.05f;
        m_errorPerYear = errorPerYear;
        m_daysAsError = daysAsError;
        m_currentSolutions = 0;

        for ( int i = 0; i <= k_maxConditionYear + 1; ++i )
        {
            m_conditionsEachNYears[ i ] = true;
            m_conditionsYearsEndIn[ i ] = true;
        }
    }

    void SolverIteration(Calendar &calendar, int iteration, float prevErrorPercentage)
    {
        CalendarInfo solution;
        calendar.ApplyCorrections();
        calendar.GetCurrentCalendarInfo( solution );
        float errorPercentage = solution.m_errorPercentage;

        if ( errorPercentage > prevErrorPercentage )
        {
            return;
        }

        if ( solution.GoodEnough( m_errorThreshold ) )
        {
            TryToAddSolution( solution );
        }

        if ( iteration >= k_maxIterations )
        {
            return;
        }

        Node node;

        for ( int i = 2; i < k_maxConditionYear; ++i )
        {
            if ( m_conditionsEachNYears[ i ] )
            {
                m_conditionsEachNYears[ i ] = false;
                node.Set( i, 1.0f );
                calendar.AddCorrectionEachNYears( node );
                SolverIteration( calendar, iteration + 1, errorPercentage );
                calendar.RemoveCorrectionEachNYears();
                m_conditionsEachNYears[ i ] = true;
            }
        }

        if ( iteration == 0 )
        {
            printf( ">> 25% completed" );
        }

        for ( int i = 3; i < k_maxConditionYear; ++i )
        {
            if ( m_conditionsEachNYears[ i ] )
            {
                m_conditionsEachNYears[ i ] = false;
                node.Set( i, -1.0f );
                calendar.AddCorrectionEachNYears( node );
                SolverIteration( calendar, iteration + 1, errorPercentage );
                calendar.RemoveCorrectionEachNYears();
                m_conditionsEachNYears[ i ] = true;
            }
        }

        if ( iteration == 0 )
        {
            printf( ">> 50% completed" );
        }

        //

        //for ( int i = 3; i < k_maxConditionYear; ++i )
        //{
        //    if ( m_conditionsYearsEndIn[ i ] )
        //    {
        //        m_conditionsYearsEndIn[ i ] = false;
        //        node.Set( i, 1.0f );
        //        calendar.AddCorrectionYearsEndIn( node );
        //        SolverIteration( calendar, iteration + 1, errorPercentage );
        //        calendar.RemoveCorrectionYearsEndIn();
        //        m_conditionsYearsEndIn[ i ] = true;
        //    }
        //}
        //
        //if ( iteration == 0 )
        //{
        //    printf( ">> 75% completed" );
        //}
        //
        //for ( int i = 2; i < k_maxConditionYear; ++i )
        //{
        //    if ( m_conditionsYearsEndIn[ i ] )
        //    {
        //        m_conditionsYearsEndIn[ i ] = false;
        //        node.Set( i, -1.0f );
        //        calendar.AddCorrectionYearsEndIn( node );
        //        SolverIteration( calendar, iteration + 1, errorPercentage );
        //        calendar.RemoveCorrectionYearsEndIn();
        //        m_conditionsYearsEndIn[ i ] = true;
        //    }
        //}
        //
        //if ( iteration == 0 )
        //{
        //    printf( ">> 100% completed" );
        //}
    }

    void TryToAddSolution(CalendarInfo &solution)
    {
        if ( m_currentSolutions == 0 )
        {
            m_solutions[ m_currentSolutions++ ] = solution;
            m_solutions[ 0 ].ShowCalendarInfo();
        }
        else
        {
            int i = m_currentSolutions - 1;
            int j = i - 1;
            while ( j >= 0 && solution < m_solutions[ j ] )
            {
                --i;
                --j;
            }
            InsertSolution( solution, i );
        }
    }

    void InsertSolution( CalendarInfo &solution, int index )
    {
        if ( m_currentSolutions + 1 < k_maxSolutions )
        {
            ++m_currentSolutions;
        }

        for ( int i = m_currentSolutions - 1; i > index; --i )
        {
            m_solutions[ i ] = m_solutions[ i - 1 ];
        }
        m_solutions[ index ] = solution;

        if ( m_currentSolutions == k_maxSolutions )
        {
            m_errorThreshold = m_solutions[ m_currentSolutions - 1 ].m_errorPercentage;
        }
        solution.ShowCalendarInfo();
    }

};

int main()
{
    float errorPerYear = 1.73128425136941f - 2.0f;
    float daysAsError = 2.0f;
    
    //CalendarSolver solver( errorPerYear, daysAsError );
    //solver.Solver();
    //solver.ShowSolutions();

    Calendar calendar( errorPerYear, daysAsError );

    Node node;
    node.Set( 3, 1.0f );
    calendar.AddCorrectionEachNYears( node );
    node.Set( 15, -1.0f );
    calendar.AddCorrectionEachNYears( node );
    node.Set( 488, 1.0f );
    calendar.AddCorrectionEachNYears( node );
    calendar.ApplyCorrections();

    CalendarInfo calendarInfo;
    calendar.GetCurrentCalendarInfo( calendarInfo );
    calendarInfo.ShowCalendarInfo();

    system( "pause" );

    return 0;
}