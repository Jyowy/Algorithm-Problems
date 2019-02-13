#include <cstdio>
#include <queue>
#include <stack>
#include <cmath>
#include <vector>

// https://www.codingame.com/training/hard/the-labyrinth

enum Direction
{
    UP = 0,
    LEFT,
    RIGHT,
    DOWN,
    DSIZE
};

const char *directionStr[ DSIZE ] =
{
    "UP",
    "LEFT",
    "RIGHT",
    "DOWN"
};

struct Point
{
    int x;
    int y;
    Point( int a, int b ) : x( a ), y( b ) {}
    void SetPoint( int a, int b ) { x = a; y = b; }

    bool operator==( const Point &other ) const { return x == other.x && y == other.y; }
};

class Map
{
public:
    Map( char *buffer, int width, int height )
        : m_map( buffer ), m_size( width, height )
        , m_realSize( height * width + height )
        , m_offset( width + 1 )
        , m_state( START )
        , m_kirk( -1, -1 )
        , m_startPosition( -1, -1 )
        , m_controlRoom( -1, -1 )
    {
        fprintf( stderr, "## Map\nSize: %d, offset: %d\n##\n", m_realSize, m_offset );
        m_parents = new int[ m_realSize ];
        m_pathSize = new int[ m_realSize ];
    }
    ~Map()
    {
        delete[] m_parents;
        delete[] m_pathSize;
        m_map = 0;
    }

    void Update();
    Direction GetStep()
    {
        Direction direction = m_path.front();
        m_path.pop();
        return direction;
    }

private:

    enum State
    {
        START = 0,
        SEARCHING_CONTROL_ROOM,
        GOING_TO_CONTROL_ROOM,
        TRYING_TO_LEAVE,
        LEAVING
    } m_state;

    struct Node
    {
        int position;
        int value;

        bool operator<( const Node &other ) const { return value > other.value; }
    };

    void LookForControlRoom();
    int GetNodeValue( const Point &position, const Point &target, int pathSize ) const;
    bool CalculatePath( const Point &target );
    int CoordToPosition( int x, int y ) const { return y * m_offset + x; }
    void PositionToCoord( int &x, int &y, int pos ) const
    {
        y = pos / m_offset;
        x = pos - y * m_offset;
    }
    Direction GetDirection( int pos1, int pos2 ) const
    {
        Direction direction = UP;
        int diff = std::abs( pos1 - pos2 );
        if ( diff == 1 )
        {
            direction = pos1 < pos2 ? RIGHT : LEFT;
        }
        else
        {
            direction = pos1 < pos2 ? DOWN : UP;
        }
        return direction;
    }

    Point m_size;
    int m_realSize;
    int m_offset;
    int *m_parents;
    int *m_pathSize;
    char *m_map;

    Point m_kirk;
    Point m_startPosition;
    Point m_controlRoom;
    std::queue<Direction> m_path;
};

void Map::Update()
{
    scanf( "%d%d%*c", &m_kirk.y, &m_kirk.x );
    fread( m_map, sizeof( char ), m_realSize, stdin );

    fprintf( stderr, "Kirk position: %d, %d (%d): '%c'\n", m_kirk.x, m_kirk.y, CoordToPosition( m_kirk.x, m_kirk.y ), m_map[ CoordToPosition( m_kirk.x, m_kirk.y ) ] );
    fwrite( m_map, sizeof( char ), m_realSize, stderr );

    switch ( m_state )
    {
    case START:
        fprintf( stderr, "Initialize map\n" );
        m_startPosition.x = m_kirk.x;
        m_startPosition.y = m_kirk.y;

        m_state = SEARCHING_CONTROL_ROOM;

    case SEARCHING_CONTROL_ROOM:

        fprintf( stderr, "Looking for control room\n" );
        if ( m_controlRoom.x < 0 )
        {
            for ( int i = 0; i < m_realSize; ++i )
            {
                if ( m_map[ i ] == 'C' )
                {
                    PositionToCoord( m_controlRoom.x, m_controlRoom.y, i );
                    break;
                }
            }
        }

        if ( m_controlRoom.x < 0 )
        {
            fprintf( stderr, "Control room not found\n" );
            // Still don't know where the control room is located
            LookForControlRoom();
        }
        else
        {
            fprintf( stderr, "Control room found\n" );
            bool pathFound = CalculatePath( m_controlRoom );
            if ( pathFound )
            {
                m_state = GOING_TO_CONTROL_ROOM;
            }
        }

        break;

    case GOING_TO_CONTROL_ROOM:

        if ( m_kirk == m_controlRoom )
        {
            bool pathFound = CalculatePath( m_startPosition );
            m_state = pathFound ? LEAVING : TRYING_TO_LEAVE;
        }

        break;

    case TRYING_TO_LEAVE:

        if ( CalculatePath( m_startPosition ) )
        {
            m_state = LEAVING;
        }
        break;

    case LEAVING:
        break;

    }
}

void Map::LookForControlRoom()
{
    fprintf( stderr, "Trying to find control room\n" );
    std::queue<int> nodes;
    bool pathFound = false;
    Point target( -1, -1 );
    Point currentPos(-1, -1);

    for ( int i = 0; i < m_realSize; ++i ) m_parents[i] = -1;
    int position = CoordToPosition( m_kirk.x, m_kirk.y );
    m_parents[ position ] = position;
    nodes.push( position );

    int neighbors[ DSIZE ];
    neighbors[ UP ] = -m_offset;
    neighbors[ LEFT ] = -1;
    neighbors[ RIGHT ] = 1;
    neighbors[ DOWN ] = m_offset;

    while ( !nodes.empty() && target.x < 0 )
    {
        int pos = nodes.front();
        nodes.pop();

        int pathSize = m_pathSize[ pos ];
        for ( int d = UP; d < DSIZE; ++d )
        {
            int p = pos + neighbors[ d ];
            if ( m_parents[ p ] < 0 )
            {
                if ( m_map[ p ] == '?' )
                {
                    PositionToCoord( target.x, target.y, p );
                    break;
                }
                else if ( m_map[ p ] != '#' )
                {
                    nodes.push( p );
                    m_parents[ p ] = pos;
                }
            }
        }
    }

    fprintf( stderr, "Nearest '?' found at (%d, %d) (%d)\n", target.x, target.y, CoordToPosition( target.x, target.y ) );
    PositionToCoord( currentPos.x, currentPos.y, CoordToPosition( target.x, target.y ) );
    CalculatePath( target );
}

int Map::GetNodeValue( const Point &position, const Point &target, int pathSize ) const
{
    int value = -1;
    int pos = CoordToPosition( position.x, position.y );
    
    if ( position == target ) return 0;
    if ( m_map[ pos ] != '#' &&  m_pathSize[ pos ] > pathSize + 1 && m_parents[ pos ] < 0 )
    {
        // calculate euristic value
        value = std::abs( position.x - target.x ) + std::abs( position.y - target.y );
        value += pathSize + 1;
        if ( m_map[ pos ] != '?' ) value += 5;
    }

    return value;
}

bool Map::CalculatePath( const Point &target )
{
    bool found = false;

    std::priority_queue < Node > nodeQueue;
    Node node;
    node.position = CoordToPosition( m_kirk.x, m_kirk.y );
    node.value = 0;
    nodeQueue.push( node );

    int src = CoordToPosition( m_kirk.x, m_kirk.y );
    int dst = CoordToPosition( target.x, target.y );

    for ( int i = 0; i < m_realSize; ++i ) m_parents[ i ] = -1;
    m_parents[ src ] = src;
    for ( int i = 0; i < m_realSize; ++i ) m_pathSize[ i ] = m_realSize;
    m_pathSize[ src ] = 0;

    Point point(-1, -1);
    int neighbors[ DSIZE ];
    neighbors[ UP ] = -m_offset;
    neighbors[ LEFT ] = -1;
    neighbors[ RIGHT ] = 1;
    neighbors[ DOWN ] = m_offset;

    fprintf( stderr, ">>\nStart calculating path\nKirk pos: %d\nTarget: %d\n>>\n", src, dst );

    // find the shortest path to the target
    while ( !nodeQueue.empty() )
    {
        int pos = 0;
        {
            const Node &n = nodeQueue.top();
            pos = n.position;
            fprintf( stderr, "Node (pos = %d) expanded with value: %d\n", pos, n.value );
            nodeQueue.pop();
        }

        if ( pos == dst )
        {
            found = true;
            break;
        }

        int pathSize = m_pathSize[ pos ];
        for ( int d = UP; d < DSIZE; ++d )
        {
            int p = pos + neighbors[ d ];
            PositionToCoord( point.x, point.y, p );
            int v = GetNodeValue( point, target, pathSize );
            if ( v >= 0 )
            {
                m_parents[ p ] = pos;
                m_pathSize[ p ] = pathSize + 1;
                node.position = p;
                node.value = v;

                nodeQueue.push( node );
            }
        }
    }
    fprintf( stderr, "Path found\n" );
    // retrieve the path found
    int pos = dst;
    std::stack < Direction > stepStack;
    while ( pos != src )
    {
        if ( m_map[ pos ] == '?' ) found = false;
        int parent = m_parents[ pos ];
        Direction direction = GetDirection( parent, pos );
        stepStack.push( direction );
        pos = parent;
    }
    fprintf( stderr, "Path calculated:\n##\n" );
    while ( !m_path.empty() ) m_path.pop();
    while ( !stepStack.empty() )
    {
        fprintf( stderr, "%s ", directionStr[ stepStack.top() ] );
        m_path.push( stepStack.top() );
        stepStack.pop();
    }
    fprintf( stderr, "\n##\n" );

    return found;
}

int main()
{
    int height;
    int width;
    int alarmTimer;
    scanf( "%d%d%d", &height, &width, &alarmTimer );
    fprintf( stderr, "H: %d, W: %d, A: %d\n", height, width, alarmTimer );

    unsigned int size = height * width + height;
    char *buffer = new char[ size + 1 ];
    Map shipMap( buffer, width, height );

    // game loop
    while ( 1 )
    {
        shipMap.Update();
        Direction nextDirection = shipMap.GetStep();
        const char *nextDirectionStr = directionStr[ nextDirection ];
        printf( "%s\n", nextDirectionStr );
    }

    delete[] buffer;
    return 0;
}