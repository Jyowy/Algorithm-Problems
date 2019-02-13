#include <cstdio>
#include <vector>
#include <queue>

// https://www.codingame.com/training/hard/skynet-revolution-episode-2

struct Node
{
    Node( int N ) : m_isGateway( false ), m_edges( 0 )
    {
        m_edges = new bool[ N ];
        for ( int i = 0; i < N; ++i ) m_edges[ i ] = false;
    }
    ~Node() { delete[] m_edges; }
    void SetGateway() { m_isGateway = true; }
    void AddEdge( int node )
    {
        m_edges[ node ] = true;
    }
    void RemoveEdge( int node )
    {
        m_edges[ node ] = false;
    }

    bool *m_edges;
    bool m_isGateway;
};

class Network
{
public:

    Network( int size )
        : m_size( size ), m_nodes( 0 ), m_visited( 0 )
    {
        m_nodes = new Node *[ m_size ];
        for ( int i = 0; i < m_size; ++i ) m_nodes[ i ] = new Node( m_size );
        m_visited = new bool[ m_size ];
        for ( int i = 0; i < m_size; ++i ) m_visited[ i ] = false;
    }
    ~Network()
    {
        for ( int i = 0; i < m_size; ++i ) delete m_nodes[ i ];
        delete[] m_nodes;
        delete[] m_visited;
    }

    void AddEdge( int node1, int node2 )
    {
        m_nodes[ node1 ]->AddEdge( node2 );
        m_nodes[ node2 ]->AddEdge( node1 );
    }

    void RemoveEdge( int node1, int node2 )
    {
        m_nodes[ node1 ]->RemoveEdge( node2 );
        m_nodes[ node2 ]->RemoveEdge( node1 );

        printf( "%d %d\n", node1, node2 );
    }

    void SetGateway( int node ) { m_nodes[ node ]->SetGateway(); }

    void BlockBestPath( int start );

private:

    struct PathNode
    {
        int node;
        int deep;
        int cost;
    };

    int m_size;
    Node **m_nodes;
    bool *m_visited;
};

void Network::BlockBestPath( int start )
{
    std::pair<int, int> edgeToRemove( -1, -1 );
    for ( int i = 0; i < m_size; ++i )
    {
        m_visited[ i ] = false;
    }
    m_visited[ start ] = true;

    std::queue<PathNode> nodes;
    PathNode pathNode;
    bool *edges = m_nodes[ start ]->m_edges;
    for ( int i = 0; i < m_size; ++i )
    {
        if ( edges[ i ] )
        {
            if ( m_nodes[ i ]->m_isGateway )
            {
                RemoveEdge( start, i );
                return;
            }
            else
            {
                pathNode.node = i;
                pathNode.deep = 0;
                pathNode.cost = 0;
                nodes.push( pathNode );
                m_visited[ i ] = true;
            }
        }
    }

    bool stop = false;
    bool doubleFound = false;
    int moves = m_size;
    while ( !nodes.empty() && !stop )
    {
        const PathNode node = nodes.front();
        nodes.pop();
        int n = node.node;
        edges = m_nodes[ n ]->m_edges;
        int ngateways = 0;
        int edge = -1;
        for ( int i = 0; i < m_size; ++i )
        {
            if ( edges[ i ] && m_nodes[ i ]->m_isGateway )
            {
                ngateways++;
                edge = i;
            }
        }

        int deep = node.deep + 1;
        int cost = node.cost + ngateways;
        int diff = deep - cost;

        if ( diff < moves && edge > 0 )
        {
            moves = diff;
            edgeToRemove.first = n;
            edgeToRemove.second = edge;
        }
        for ( int i = 0; i < m_size; ++i )
        {
            if ( edges[ i ] && !m_nodes[ i ]->m_isGateway && !m_visited[ i ] )
            {
                m_visited[ i ] = true;
                pathNode.node = i;
                pathNode.deep = deep;
                pathNode.cost = cost;
                nodes.push( pathNode );
            }
        }

    }

    RemoveEdge( edgeToRemove.first, edgeToRemove.second );
    return;
}

int main()
{
    int N;
    int L;
    int E;
    scanf( "%d%d%d", &N, &L, &E );

    Network network( N );

    for ( int i = 0; i < L; i++ )
    {
        int N1;
        int N2;
        scanf( "%d%d", &N1, &N2 );
        network.AddEdge( N1, N2 );
    }
    for ( int i = 0; i < E; i++ )
    {
        int EI;
        scanf( "%d", &EI );
        network.SetGateway( EI );
    }

    while ( 1 )
    {
        int SI;
        scanf( "%d", &SI );
        network.BlockBestPath( SI );
    }
}