#include <cstdio>
#include <queue>
#include <vector>

// https://www.codingame.com/training/hard/the-bridge-episode-2

enum Operation
{
    SPEED,
    JUMP,
    WAIT,
    SLOW,
    UP,
    DOWN,
    OSIZE
};

const char *k_operationStr[8] =
{
    "SPEED",
    "JUMP",
    "WAIT",
    "SLOW",
    "UP",
    "DOWN"
};

class IA
{
public:

    IA( int nMotorbikes, int nMotorbikesNeeded )
        : m_map( 0 )
        , m_offset( 0 )
        , m_nMotorbikes( nMotorbikes )
        , m_nMotorbikesNeeded( nMotorbikesNeeded )
    {
        m_map = new char[ 4 * 500 ];
    }
    ~IA()
    {
        delete[] m_map;
    }

    void ReadMap()
    {
        scanf( "%s%*c", m_map );
        m_offset = 0;
        while ( m_map[ m_offset++ ] != '\0' );
        m_map[ m_offset - 1 ] = '\n';
        m_laneSize = m_offset - 1;
        m_size = m_offset * 4;
        fread( &m_map[ m_offset ], sizeof( char ), m_offset * 3, stdin );

        fprintf( stderr, "Map (4 x %d = %d):\n", m_offset, m_size );
        fwrite( m_map, sizeof( char ), m_size, stderr );
    }

    void Update()
    {
        static int turn = 1;
        scanf( "%d", &m_speed );
        fprintf( stderr, "# Turn %d\n", turn++ );
        fprintf( stderr, "Current speed: %d\n", m_speed );
        for ( int i = 0; i < m_nMotorbikes; ++i )
        {
            scanf( "%d%d%d", &m_motorbikes[ i ].m_x, &m_motorbikes[ i ].m_y, &m_motorbikes[ i ].m_isActive );
            fprintf( stderr, "> %d, %d, %d\n", m_motorbikes[ i ].m_x, m_motorbikes[ i ].m_y, m_motorbikes[ i ].m_isActive );
        }
        fprintf( stderr, "***\n" );
    }

    void CalculateOperations();

    Operation GetNextOperation()
    {
        Operation operation = m_operations.front();
        m_operations.pop();
        return operation;
    }

private:

    bool Backtracking( std::vector<Operation> &operations, int idx, int pos, int speed, std::vector<int> lanes, std::vector<bool> active, int nActive );

    struct Motorbike
    {
        int m_x;
        int m_y;
        int m_isActive;
    };

    struct Node
    {
        int m_jump;
        int m_motorbikes;
    };

    char *m_map;
    int m_laneSize;
    int m_size;
    int m_offset;
    int m_speed;
    int m_nMotorbikes;
    int m_nMotorbikesNeeded;
    Motorbike m_motorbikes[ 4 ];
    std::queue<Operation> m_operations;
};

bool IA::Backtracking( std::vector<Operation> &operations, int idx, int pos, int speed, std::vector<int> lanes, std::vector<bool> active, int nActive )
{
    bool end = false;
    std::vector<bool> activeBU(active.begin(), active.end());

    if ( pos >= m_laneSize - 1 ) return true;

    for ( int i = 0; !end && i < OSIZE; ++i )
    {
        bool ok = true;
        int na = nActive;
        int s = speed;
        switch ( i )
        {
        case SPEED:
            s++;
        case WAIT:
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    int p = lanes[ m ] * m_offset + pos;
                    for ( int j = 1; j <= s; ++j )
                    {
                        if ( pos + j >= m_laneSize - 1 )
                        {
                            break;
                        }
                        if ( m_map[ p + j ] == '0' )
                        {
                            ok = false;
                            break;
                        }
                    }
                    if ( !ok )
                    {
                        active[ m ] = false;
                        na--;
                        if ( na < m_nMotorbikesNeeded ) break;
                    }
                }
            }
            if ( na >= m_nMotorbikesNeeded )
            {
                operations.push_back( s > speed ? SPEED : WAIT );
                end = Backtracking( operations, idx + 1, pos + s, s, lanes, active, na );
                if ( !end ) operations.pop_back();
            }
            break;

        case JUMP:
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    int p = lanes[ m ] * m_offset + pos + s;
                    if ( pos + s >= m_laneSize )
                    {
                        break;
                    }
                    if ( m_map[ p ] == '0' )
                    {
                        active[ m ] = false;
                        na--;
                        if ( na < m_nMotorbikesNeeded ) break;
                    }
                }
            }
            if ( na >= m_nMotorbikesNeeded )
            {
                operations.push_back( JUMP );
                end = Backtracking( operations, idx + 1, pos + s, s, lanes, active, na );
                if ( !end ) operations.pop_back();
            }
            break;

        case SLOW:
            if ( speed <= 1 )
            {
                break;
            }
            s--;
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    int p = lanes[ m ] * m_offset + pos;
                    for ( int j = 1; j <= s; ++j )
                    {
                        if ( pos + j >= m_laneSize )
                        {
                            break;
                        }
                        if ( m_map[ p + j ] == '0' )
                        {
                            ok = false;
                            break;
                        }
                    }
                    if ( !ok )
                    {
                        active[ m ] = false;
                        na--;
                        if ( na < m_nMotorbikesNeeded ) break;
                    }
                }
            }
            if ( na >= m_nMotorbikesNeeded )
            {
                operations.push_back( SLOW );
                end = Backtracking( operations, idx + 1, pos + s, s, lanes, active, na );
                if ( !end ) operations.pop_back();
            }
            break;

        case UP:
            ok = true;
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    ok = lanes[ m ] > 0;
                    break;
                }
            }
            if ( !ok )
            {
                break;
            }
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    int p = lanes[ m ] * m_offset + pos;
                    for ( int j = 1; j < s; ++j )
                    {
                        if ( pos + j >= m_laneSize )
                        {
                            break;
                        }
                        if ( m_map[ p - m_offset + j ] == '0' || m_map[ p + j ] == '0' )
                        {
                            ok = false;
                            break;
                        }
                    }
                    p += s - m_offset;
                    if ( !ok || m_map[ p ] == '0' )
                    {
                        active[ m ] = false;
                        na--;
                        if ( na < m_nMotorbikesNeeded ) break;
                    }
                }
            }
            if ( na >= m_nMotorbikesNeeded )
            {
                for ( int m = 0; m < m_nMotorbikes; ++m )
                {
                    lanes[ m ]--;
                }
                operations.push_back( UP );
                end = Backtracking( operations, idx + 1, pos + s, s, lanes, active, na );
                if ( !end )
                {
                    operations.pop_back();
                    for ( int m = 0; m < m_nMotorbikes; ++m ) lanes[ m ]++;
                }
            }
            break;

        case DOWN:
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m_nMotorbikes - 1 - m ] )
                {
                    ok = lanes[ m ] < 3;
                    break;
                }
            }
            if ( !ok )
            {
                break;
            }
            for ( int m = 0; m < m_nMotorbikes; ++m )
            {
                if ( active[ m ] )
                {
                    int p = lanes[ m ] * m_offset + pos;
                    for ( int j = 1; j < s; ++j )
                    {
                        if ( pos + j >= m_laneSize )
                        {
                            break;
                        }
                        if ( m_map[ p + j ] == '0' || m_map[ p + m_offset + j ] == '0' )
                        {
                            ok = false;
                            break;
                        }
                    }
                    p += s + m_offset;
                    if ( !ok || m_map[ p ] == '0' )
                    {
                        active[ m ] = false;
                        na--;
                        if ( na < m_nMotorbikesNeeded ) break;
                    }
                }
            }
            if ( na >= m_nMotorbikesNeeded )
            {
                for ( int m = 0; m < m_nMotorbikes; ++m )
                {
                    if ( active[ m ] ) lanes[ m ]++;
                }
                operations.push_back( DOWN );
                end = Backtracking( operations, idx + 1, pos + s, s, lanes, active, na );
                if ( !end )
                {
                    operations.pop_back();
                    for ( int m = 0; m < m_nMotorbikes; ++m ) lanes[ m ]--;
                }
            }
            break;
        }

        for ( int m = 0; m < m_nMotorbikes; ++m )
        {
            active[ m ] = activeBU[ m ];
        }
    }

    return end;
}

void IA::CalculateOperations()
{
    std::vector<Operation> operations;
    std::vector<bool> active;
    int startX = 0;
    if ( m_speed == 0 )
    {
        operations.push_back( SPEED );
        m_speed = 1;
        startX = 1;
    }
    active.resize( m_nMotorbikes );
    for ( int i = 0; i < m_nMotorbikes; ++i ) active[ i ] = m_motorbikes[i].m_isActive;
    std::vector<int> lanes;
    lanes.resize( m_nMotorbikes );
    for ( int i = 0; i < m_nMotorbikes; ++i ) lanes[ i ] = m_motorbikes[ i ].m_y;
    Backtracking( operations, 0, startX, m_speed, lanes, active, m_nMotorbikes );

    fprintf( stderr, "Operaciones:\n" );
    for ( int i = 0; i < operations.size(); ++i )
    {
        Operation operation = operations[ i ];
        fprintf( stderr, "%d) %s\n", i, k_operationStr[ operation ] );
        m_operations.push( operation );
    }

    return;
}

int main()
{
    int M;
    int V;
    scanf( "%d%d*c", &M, &V );
    fprintf( stderr, "%d, %d\n", M, V );
    IA ia( M, V );
    ia.ReadMap();
    ia.Update();
    ia.CalculateOperations();

    while ( 1 )
    {
        Operation operation = ia.GetNextOperation();
        printf( "%s\n", k_operationStr[ operation ] );
        ia.Update();
    }
}