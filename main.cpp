#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <math.h>
#include <queue>
#include <unordered_set>
#include <functional>
#include <thread>
#include <future>
#include <climits>
#include <algorithm>
#include <random>

using namespace sf;
using namespace std;

//variables
float hex_rad=(2*900)/(9*3+1), unit;
int turn=0;
float n=6;
ConvexShape border_unit(3);
CircleShape circ(hex_rad*sqrt(3)/2-5);
Color blue = Color(66, 211, 255, 255);
vector<pair<int, int>> ai_move_checklist;
int max_threads=10;
Font font;
RenderWindow window(sf::VideoMode(1550, 900), "Hex");
vector<vector<int>> board(21, vector<int>(21, 0));
void ai_pair(vector<vector<int>> board, pair<int, int>&move,int depth, int turn);
void makeMove( int i, int j);
namespace mcts{struct Node;
void deleteNodes(mcts::Node*&node);}

bool swap_happened=false;

//functions
void setpos_borderunit(float x, float y, string where);
bool f(int i, int j, int target, vector<vector<bool>>& visited);
void vv(string hay);
bool mouse_in_shape(Vector2i mouse, ConvexShape con);
void swapAct(vector<vector<int>>&board);
void resize();
void ww(float a);
//int check_for_winner(vector<vector<int>> board);
//void ww(float a);

void refreshTextTurn();
pair<int, int> ai_pair(vector<vector<int>> board, int depth, int turn);

enum ai_var
{
	human=0,
	easy=1,
	medium=2,
	hard=3
};
ai_var ai_selection[2];

enum state_var{READY, WORKING, DONE};
enum game_stage_var
{
	start_menu,
	game
};
game_stage_var game_stage = start_menu;

class Table
{
	public:
	CircleShape hex, circ;
	ConvexShape border_left, border_right, border_top, border_bottom;
	Table()
	{
		hex.setPointCount(6);
		hex.setOutlineColor(Color::Black);

		border_right.setPointCount(3);
		border_left.setPointCount(3);
		border_top.setPointCount(3);
		border_bottom.setPointCount(3);

		border_top.setFillColor(Color::Blue);
		border_bottom.setFillColor(Color::Blue);
		border_right.setFillColor(Color::Red);
		border_left.setFillColor(Color::Red);
	}
	void resize()
	{
		hex.setRadius(hex_rad);
		hex.setOrigin(hex.getPoint(0));
		hex.setOutlineThickness(-hex_rad/20);

		circ.setRadius(hex_rad*sqrt(3)/2-hex_rad/10);
		circ.setOrigin(circ.getPoint(0).x, circ.getPoint(0).y+circ.getRadius());
		
		border_top.setOrigin(0, 0);
		border_top.setPoint(0, Vector2f(0, 0));
		border_top.setPoint(1, Vector2f(hex_rad*sqrt(3), 0));
		border_top.setPoint(2, Vector2f(hex_rad*sqrt(3)/2, hex_rad/2));

		border_bottom.setOrigin(0, 0);
		border_bottom.setPoint(0, Vector2f(-hex_rad*sqrt(3)/2, 1.5*hex_rad));
		border_bottom.setPoint(1, Vector2f(-hex_rad*sqrt(3), 2*hex_rad));
		border_bottom.setPoint(2, Vector2f(0, 2*hex_rad));

		border_left.setOrigin(0, 0);
		border_left.setPoint(0, Vector2f(-hex_rad*sqrt(3)/2, hex_rad/2));
		border_left.setPoint(1, Vector2f(-hex_rad*sqrt(3)/2, 1.5*hex_rad));
		border_left.setPoint(2, Vector2f(-hex_rad*sqrt(3), hex_rad*2));

		border_right.setOrigin(0, 0);
		border_right.setPoint(0, Vector2f(hex_rad*sqrt(3)/2, hex_rad/2));
		border_right.setPoint(1, Vector2f(hex_rad*sqrt(3)/2, hex_rad*1.5));
		border_right.setPoint(2, Vector2f(hex_rad*sqrt(3), 0));
	}
	void draw()
	{
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				hex.setPosition(Vector2f( hex_rad*sqrt(3)/2 +(n-i)*hex_rad*sqrt(3)/2+j*hex_rad*sqrt(3) , i*(1.5*hex_rad)));
				window.draw(hex);
				if( abs(board[i][j])==1 )
				{
					if(board[i][j]==1)
					{
						circ.setFillColor(Color::Blue);
					}
					else if(board[i][j]==-1)
					{
						circ.setFillColor(Color::Red);
					}
					circ.setPosition(hex.getPosition().x, hex.getPosition().y+hex_rad);
					window.draw(circ);
				}

				if(i==0)
				{
					border_top.setPosition(hex.getPosition());
					window.draw(border_top);
				}
				if(i==n-1)
				{
					border_bottom.setPosition(hex.getPosition());
					window.draw(border_bottom);
				}
				if(j==0)
				{
					border_left.setPosition(hex.getPosition());
					window.draw(border_left);
				}
				if(j==n-1)
				{
					border_right.setPosition(hex.getPosition());
					window.draw(border_right);
				}
			}
		}

	}
}
table;

class Button
{
	public:
	ConvexShape hex;
	Text text;
	void setString(String str,Color c_text, Color c_hex)
	{
		text.setString(str);
		text.setFillColor(c_text);
		hex.setFillColor(c_hex);
		font.loadFromFile("typo.otf");
		text.setFont(font);
		hex.setPointCount(6);
	}
	Button(String str,Color c_text, Color c_hex)
	{
		setString(str, c_text, c_hex);
	}
	void resize(float x, float y, float text_size, float b=sqrt(3)/2)
	{
		float a=0.1*unit;
		text.setCharacterSize(text_size);
		text.setOrigin(Vector2f(text.getLocalBounds().getPosition().x+text.getGlobalBounds().width/2 , text.getLocalBounds().getPosition().y+text.getGlobalBounds().height/2));
		text.setPosition(x, y);
		
		hex.setPoint(0, Vector2f(text.getPosition().x-text.getGlobalBounds().width/2- a, text.getPosition().y-text_size-a));
		hex.setPoint(1, Vector2f(text.getPosition().x+text.getGlobalBounds().width/2+ a, text.getPosition().y-text_size-a));
		hex.setPoint(2, Vector2f(text.getPosition().x+text.getGlobalBounds().width/2+ a + (2*a+text_size)*b , text.getPosition().y));

		hex.setPoint(3, Vector2f(text.getPosition().x+text.getGlobalBounds().width/2+ a , text.getPosition().y+text_size+a));
		hex.setPoint(4, Vector2f(text.getPosition().x-text.getGlobalBounds().width/2- a , text.getPosition().y+text_size+a));
		hex.setPoint(5, Vector2f(text.getPosition().x-text.getGlobalBounds().width/2- a - (2*a+text_size)*b, text.getPosition().y));
		hex.setOrigin((hex.getPoint(2).x+hex.getPoint(5).x)/2, hex.getPoint(2).y);
		hex.setPosition(x, y);
	}
	bool eq(Vector2f a, Vector2f b)
	{
		Vector2i m=Mouse::getPosition(window);
		if( (m.y-a.y)<(b.y-a.y)/(b.x-a.x)*(m.x-a.x) )
			if(a.x<b.x) return 1;
				else return 0;

		if(a.x<b.x) return 0;
		else return 1;
	}
	bool isMouseInside()
	{
		for(int i=0;i< hex.getPointCount()-1;i++)
			if(eq(hex.getPoint(i), hex.getPoint(i+1)))
				return 0;
		if(eq(hex.getPoint( hex.getPointCount()-1 ), hex.getPoint(0)))
			return 0;
		return 1;
	}
}
Bstart("Start Game", Color(0, 0, 0, 255), Color(255, 255, 255, 255)), con("Continue", Color(0, 0, 0, 255), Color(255, 255, 255, 255)), Bswap("Swap", Color(0, 0, 0, 255), Color(255, 255, 255, 255)), Bexit("Exit", Color(0, 0, 0, 255), Color(255, 255, 255, 255));
vector<vector<Button>> ai_button=
{ {
		Button("human", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("easy", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("medium", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("hard", Color(0, 0, 0, 255), Color(255, 255, 255, 255))
	},
	{
		Button("human", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("easy", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("medium", Color(0, 0, 0, 255), Color(255, 255, 255, 255)),
		Button("hard", Color(0, 0, 0, 255), Color(255, 255, 255, 255))
} };

class TEXT
{
	public:
	Text text;
	TEXT(Color color=Color::White)
	{
		font.loadFromFile("typo.otf");
		text.setFillColor(color);
		text.setFont(font);
	}
	TEXT(String str, Color color): TEXT(color)
	{
		text.setString(str);
	}
	void resize(float x, float y, float text_size)
	{
		text.setCharacterSize(text_size);
		text.setOrigin(text.getLocalBounds().getPosition().x+text.getLocalBounds().getSize().x/2, text.getLocalBounds().getPosition().y+text.getLocalBounds().getSize().y/2);
		text.setPosition(x, y);
	}
	void resize(float x, float y, float text_size, String str)
	{
		text.setString(str);
		resize(x, y, text_size);
	}
	Vector2f getPosition()
	{
		return text.getPosition();
	}
	void setString(String str)
	{
		Vector2f t=text.getPosition();
		text.setString(str);
		text.setOrigin(text.getLocalBounds().getPosition().x+text.getLocalBounds().getSize().x/2, text.getLocalBounds().getPosition().y+text.getLocalBounds().getSize().y/2);
		text.setPosition(t);
	}
}
T_PlayerBlue("Player 1(blue):", blue), T_PlayerRed("Player 2(red):", Color::Red), T_TableSize(Color::White), T_Turn(Color::White), T_WhosTurn;

class DualTriangle
{
	public:
	ConvexShape shape1, shape2;
	void setColor(Color c1, Color c2)
	{
		shape1.setFillColor(c1);
		shape1.setOutlineColor(c2);
		shape1.setPointCount(3);
		shape2.setFillColor(c1);
		shape2.setOutlineColor(c2);
		shape2.setPointCount(3);
	}
	void resize(float x, float y, float size)
	{
		shape1.setOutlineThickness(-size*5/100);
		shape2.setOutlineThickness(-size*5/100);

		shape1.setPoint(2, Vector2f(x, y+size*sqrt(3)/2));
		shape1.setPoint(1, Vector2f(x+size/2, y));
		shape1.setPoint(0, Vector2f(x-size/2, y));

		shape2.setPoint(0, Vector2f(x-size/2, y));
		shape2.setPoint(1, Vector2f(x+size/2, y));
		shape2.setPoint(2, Vector2f(x, y-(size*sqrt(3)/2)));
	}
	bool EQ(Vector2f a, Vector2f b)
	{
		Vector2i m=Mouse::getPosition(window);
		if( (m.y-a.y)>(b.y-a.y)/(b.x-a.x)*(m.x-a.x) )
			return 1;
		return 0;
		//returns 1 if mouse is under the equation and 0 if above
	}
	int isMouseInside()
	{
		if(EQ(shape1.getPoint(0), shape1.getPoint(1))==0)
		{
			if( EQ(shape2.getPoint(0), shape2.getPoint(2))==1 and EQ(shape2.getPoint(2), shape2.getPoint(1))==1 )
			{
				return 1;
			}
		}
		else
		{
			if( EQ(shape1.getPoint(0), shape1.getPoint(2))==0 and EQ(shape1.getPoint(2), shape1.getPoint(1))==0 )
			{
				return -1;
			}
		}
		return 0;
	}
}
triangle;




//position related functions
namespace pos
{
	bool f(int i, int j, int target, vector<vector<bool>> &visited,vector<vector<int>>board)
	{
		if(i<0 or i>n-1 or j<0 or j>n-1 or visited[i][j]==1 or board[i][j]!=target)
			return 0;
		visited[i][j]=true;

		//check for destination
		if(target==1 and i==n-1)
			return 1;

		if(target==-1 and j==n-1)
			return 1;

		//check neighbours
		if(f(i+1, j, target, visited, board)==1)
			return 1;
		if(f(i-1, j, target, visited, board)==1)
			return 1;
		if(f(i, j+1, target, visited, board)==1)
			return 1;
		if(f(i, j-1, target, visited, board)==1)
			return 1;
		if(f(i+1, j+1, target, visited, board)==1)
			return 1;
		if(f(i-1, j-1, target, visited, board)==1)
			return 1;

		return 0;
	}

	bool start_backtracking(int i, int j, int target, vector<vector<int>>board)
	{
		vector<vector<bool>> visited(n, vector<bool>(n));
		return f(i, j, target, visited, board);
	}

	int checkForWinner(vector<vector<int>> board)
	{
		for(int i=0;i<n;i++)
		{
			if(start_backtracking(0, i, 1, board))
			return 1;
			if(start_backtracking(i, 0, -1, board))
			return -1;
		}
		return 0;
	}

	/*bool valid(int x, int y)
	{
		if(x>=0 and y>=0 and x<n and y<n)
			return 1;
		return 0;
	}

	float bfs(const vector<vector<int>>&board, int target)
	{
		priority_queue<tuple<float, int, int>, vector<tuple<float, int, int>>, greater<>> q;
		vector<vector<int>>visited(n, vector<int>(n, INT_MAX));
	
		int di[6]={-1, -1, 0, 1, 1, 0};
		int dj[6]={-1, 0, 1, 1, 0, -1};
		int ei[6]={-2, -1, 1 ,2, 1, -1};
		int ej[6]={-1, 1, 2, 1, -1, -2};

		for(int i=0;i<n;i++)
		{
			if(target==1 and board[0][i]!=target*-1)
			{
				int cost= (board[0][i]==1)?0:1;
				visited[0][i]=cost;
				q.push({cost, 0, i});
			}
			if(target==-1 and board[i][0]!= target*-1)
			{
				int cost=(board[i][0]==-1)?0:1;
				visited[i][0]=cost;
				q.push({cost, i, 0});
			}
		}
		while(!q.empty())
		{
			float cost;
			int i, j;
			tie(cost, i, j)=q.top();
			//[cost, i, j]=q.top();
			q.pop();
			//cout<<endl<<"cost="<<cost<<" i="<<i<<" j="<<j<<endl;

			if(target==-1 and j==n-1)
				return cost;
			if(target==1 and i==n-1)
				return cost;

			for(int k=0;k<6;k++)
			{
				//adjecent
				int ni=i+di[k];
				int nj=j+dj[k];
				if(!valid(ni, nj) or board[ni][nj] == target*-1)
				{
					continue;
				}

				float new_cost = cost + (board[ni][nj] == target ? 0 : 1);
				if(new_cost<visited[ni][nj])
				{
					visited[ni][nj]=new_cost;
					q.push({new_cost, ni, nj});
				}

				
				//diagonal
				int mi=i+di[k]+di[(k+1)%6];
				int mj=j+dj[k]+dj[(k+1)%6];
				int ni2=i+di[(k+1)%6];
				int nj2=j+dj[(k+1)%6];
				//cout<<i<<" "<<j<<"    "<<mi<<" "<<mj<<endl;
				if(valid(mi, mj) and valid(ni2, nj2) and board[mi][mj]!=-1*target and board[ni][nj]==0 and board[ni2][nj2]==0)
				{
					new_cost = cost + ((board[mi][mj]==target)? 0.5f : 1);
					if(new_cost<visited[mi][mj])
					{
						//cout<<"diagonal push:\n i="<<i<<" j="<<j<<"    mi="<<mi<<" mj="<<mi<<endl;
						//cout<<"ni="<<ni<<" nj="<<nj<<"    ni2="<<ni2<<" nj2="<<nj2<<endl;
						visited[mi][mj]=new_cost;
						//cout<<"new:"<<new_cost<<"\n";
						q.push({new_cost, mi, mj});
					}
				}


			}
		}
		return -1;
	}

	float position_evaluation(const vector<vector<int>>&board)
	{
		int vic_check=check_for_winner(board);
		if(vic_check==1)
			return 900;
		if(vic_check==-1)
			return -900;
		
		vector<int> di = {-1, -1, 0, 1, 1, 0};
		vector<int> dj = {-1, 0, 1, 0, 0, 1};

		float cp_b=0, cp_r=0;

		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				for(int k=0;k<6;k++)
				{
					int ni=i+di[k];
					int nj=j+dj[k];
					if(valid(ni, nj) and board[i][j]==0)
					{
						if(board[ni][nj]==1)
							cp_b+=0.01;
						if(board[ni][nj]==-1)
							cp_r+=0.01;
					}
				}
			}
		}


		vector<int> di2 = {-2, -1, 1, 2, 1, -1};
		vector<int> dj2 = {-1, 1, 2, 1, -1, -2};
		
		float a=bfs(board, 1);
		float b=bfs(board, -1);
		//a=n-a;
		a=(n-a)*10+cp_b;
		//b=n-b;
		b=(n-b)*10+cp_r;
		return a-b;
	}*/


	vector<pair<int, int>> getPossibleMoves(const vector<vector<int>>&b, int turn)
	{
		vector<pair<int, int>>ret_vec;
		if(pos::checkForWinner(board)!=0)
			return ret_vec;
		bool isTurn1 = turn==1;
		for(int i=0;i<n;i++)
			for(int j=0;j<n;j++)
			{
				if(b[i][j]==0 or isTurn1)
					ret_vec.push_back({i, j});
			}
		return ret_vec;
		//
		//vector<pair<int, int>>ret_vec;
		//bool isTurn1 = turn==1;
		/*int i, j;
		if(ai_move_checklist.empty())
		{
			cerr<<"E M P T Y; !!!"<<endl;
			exit(1);
		}
		for(int l=ai_move_checklist.size()-1;l>=0;--l)
		{
			tie(i, j)=ai_move_checklist[i];
			if(b[i][j]==0 or isTurn1)
				ret_vec.push_back({i, j});
		}
		return ret_vec;*/
	}

	pair<int, int>getRandomMove(vector<vector<int>>b, int turn)
	{
		vector<pair<int,int>>possibleMoves=getPossibleMoves(b, turn);
		return possibleMoves[ rand()%possibleMoves.size() ];
	}

	void write_pos_moves(vector<vector<int>>b, int t)
	{
		vector<pair<int, int>>moves=getPossibleMoves(b, t);
		for(int i=0;i<moves.size();i++)
		{
			cout<<"("<<moves[i].first<<" "<<moves[i].second<<")"<<"   ";
		}
		cout<<endl;
	}

	vector<vector<int>> makeMove_ij(vector<vector<int>> board, int i, int j, int turn)
	{
		if(turn%2==0)
			board[i][j]=1;
		else
			board[i][j]=-1;
		//board[i][j]=turn%2*-2+1;
		return board;
	}

	void writeVector(vector<int>v)
	{
		for(int i:v)
		{
			cout<<i<<" ";
		}
	}

}

namespace mcts
{
	int simulation(vector<vector<int>>b, int turn, bool swapped)
	{
		while(turn<2)
		{
			pair<int,int>m=pos::getRandomMove(b,turn);
			b=pos::makeMove_ij(b, m.first, m.second, turn);
			turn++;
		}
		//int winner=pos::checkForWinner(b);
		//if(winner!=0) return (winner==1)?1:0;
		//cout<<"n="<<n<<endl;
		int m=(int)n;
		int n=m;
		//cout<<"m="<<m<<endl;

		int maxBlue, maxRed ,currentBlue, currentRed;
		if(swapped==false)
		{
			maxRed = n*n/2;
			maxBlue= n*n/2 + (n%2);
			
			currentRed = (turn)/2;
			currentBlue= (turn+1)/2;
		}
		else
		{
			maxRed = n*n/2 + (n%2);
			maxBlue= n*n/2;

			currentRed = (turn)/2;
			currentBlue = (turn-1)/2;
		}
		int requiredRed=maxRed-currentRed, requiredBlue=maxBlue-currentBlue;
		//cout<<"max blue,red"<<maxBlue<<" "<<maxRed<<"\ncurrent blue,red"<<currentBlue<<" "<<currentRed<<endl<<"req blu,red"<<requiredBlue<<" "<<requiredRed<<endl;
		vector<int>v;
		v.insert(v.end(), requiredRed, -1);
		v.insert(v.end(), requiredBlue, 1);
		std::random_device rd;  // Non-deterministic random number generator
		std::mt19937 g(rd());   // Seeded random number generator
		std::shuffle(v.begin(), v.end(), g); // Shuffle the vector

		int k=0;
		for(int i=0;i<n;i++)
		{
			for(int j=0;j<n;j++)
			{
				if(b[i][j]==0)
				{
					b[i][j]=v[k];
					k++;
				}
				//cout<<b[i][j]<<"\t";
			}
			//cout<<"\n";
		}
		//cout<<"winner="<<pos::check_for_winner(b)<<endl;
		int result;
		if (pos::checkForWinner(b)==1)
			return 1;
		else
			return 0;

		/*while(winner==0)
		{
			vector<pair<int, int>> random_moves=getPossibleMoves(b, turn);
			if (random_moves.empty()) {
				std::cerr << "ERROR: No possible moves for turn " << turn << std::endl;
				exit(1);
			}
			pair<int, int> move = random_moves[ rand()%random_moves.size() ];
			b=makeMove_ij(b, move.first, move.second, turn);
			turn++;
			winner=pos::check_for_winner(b);
		}
		result;
		if (winner==1)
			result=1;
		else
			result=0;
		cout<<"simulation="<<result<<endl;
		return result;*/
		/*
		while(pos::check_for_winner(b)==0)
		{
			vector<pair<int, int>> random_moves=getPossibleMoves(b, turn);
			pair<int, int> move = random_moves[ rand()%random_moves.size() ];
			b=makeMove_ij(b, move.first, move.second, turn);
			turn++;
		}
		return pos::check_for_winner(b);
		*/
	}

	struct Node
	{
		int wins=0;
		int visits=0;
		int turn;
		bool swapped=false;
		pair<int, int>last_move;
		vector<Node*>children;
		//unordered_map<Node*, Stats>childrenStats;
		vector<vector<int>>board;
		vector<pair<int, int>>untriedMoves;
		Node*parent;
		//vector<Node*>parents;
		
		bool isLeaf(){
			return children.empty();
		}
		float getUCT(double C = 1.8){
		if (visits == 0)
		    return numeric_limits<float>::infinity(); // Prioritize unvisited nodes
			return ( float(wins) / visits ) + C * sqrt( log(parent->visits) / visits );
		    }

		Node* selectBestUCT()
		{
			Node* best_child = nullptr;
			double best_value = -std::numeric_limits<double>::infinity();

			for (const auto& child :children) {

				double uct_value = child->getUCT();

				if (uct_value > best_value) {
					best_value = uct_value;
					best_child = child;
				}
			}
			return best_child;
		}

		/*pair<int,int> bestMove()
		{
			int max_visits=-1;
			pair<int,int>move;
			for(Node* child: children)
			{
				if(child->visits<max_visits)
				{
					max_visits=child->visits;
					move=child->last_move;
				}
			}
			return move;
		}*/

		Node(vector<vector<int>> b, int t, bool s=swap_happened,Node*p=NULL)
		{
			//parent=p;
			parent=p;
			turn=t;
			board=b;
			swapped=s;
			//create untried moves
			untriedMoves = pos::getPossibleMoves(board, turn);
		}
	}*root;

	void selectBestMove(pair<int,int>&move)
	{
		int max_visits=-1;
		Node*selectedNode;
		for(Node* child : root->children)
		{
			if(max_visits<child->visits)
			{
				max_visits=child->visits;
				selectedNode=child;
				move=child->last_move;
			}
		}
		/*
		for(Node*&child : root->children)
		{
			if(child!=selectedNode)
			{
				deleteNodes(child);
			}
		}
		delete root;
		root=selectedNode;*/
	}

	void manageRoot()
	{
		if(root==NULL)
		{
			//cout<<"\t\tcreated a new root\n";
			root= new Node(board, turn, swap_happened);
			return;
		}
		if(root->board==board)
			return;
		//cout<<"replacing the root\n";
		Node*new_root=NULL;
		for(Node*&child : root->children)
		{
			if(board==child->board)
			{
				//cout<<"selected 1\n";
				new_root=child;
			}
			else
			{
				//cout<<"deleted 1\n";
				deleteNodes(child);
			}
		}
		delete root;
		if(new_root==NULL)
		{
			//cout<<"creatinGGGGGGGGGGG\n";
			root= new Node(board, turn, swap_happened);
		}
		else
		{
			//cout<<"replacing\n";
			root=new_root;
		}
		root->parent=NULL;
	}

	void get_t(const int&n)
	{
		for(int i=0;i<n;i++)
			cout<<"\t";
	}

	void write_children(Node*node)
	{
		cout<<"visits\twins\tchildren.size\tuntriedMoves.size\tlastMove\n";
		cout<<"current node:\n"<<node->visits<<"\t"<<node->wins<<"\t"<<node->children.size()<<"\t\t"<<node->untriedMoves.size()<<"\nits children\n";
		for(Node*child:node->children)
		{
			cout<<child->visits<<"\t"<<child->wins<<"\t"<<child->children.size()<<"\t\t"<<child->untriedMoves.size()<<"\t\t\t"<<child->last_move.first<<" "<<child->last_move.second<<endl;
		}
	}

	void write_m_children(Node*node, int max_depth, int depth=0)
	{
		get_t(depth);
		cout<<node->visits<<"\t"<<node->wins<<"\t"<<node->children.size()<<"\t\t"<<node->untriedMoves.size()<<"\t\t\t"<<node->swapped<<"\t"<<node->last_move.first<<" "<<node->last_move.second<<endl;
		if(depth<max_depth)
		{
			get_t(depth+1);
			if(!node->children.empty())
				cout<<"its children:\n";
			for(Node*child:node->children)
			{
				write_m_children(child,max_depth, depth+1);
			}
			cout<<endl;
			//cout<<child->visits<<"\t"<<child->wins<<"\t"<<child->children.size()<<"\t\t"<<child->untriedMoves.size()<<"\t\t\t"<<child->last_move.first<<" "<<child->last_move.second<<endl;
		}
	}

	/*void backpropagate(Node*current, int result)
	{
		do{
			current->visits+=1;
			if(current->turn%2==1)
				current->wins += result;
			else
				current->wins += (1-result);
			
			current=current->parent;
		} while(current!=NULL);
	}*/

	void backpropagate(Node*current, int result, int visits)
	{
		do{
			current->visits+=visits;
			if(current->turn%2==1)
				current->wins += result;
			else
				current->wins += (visits-result);
			
			current=current->parent;
		} while(current!=NULL);
	}

	void runSimulation(Node*node, int&result, int&visits)
	{
		//if the node reached an endstate
		int winner = pos::checkForWinner(node->board);
		if(winner != 0)
		{
			result = (winner==1)? 10: 0;
			visits = 10;
			return;
		}
		//if wasn't visited before
		if(node->visits==0)
		{
			result=0;
			visits=5;
			vector<future<int>>thread_results;
			for(int i=0;i<5;i++)
			{
				thread_results.push_back(async(launch::async, simulation, node->board, node->turn, node->swapped));
			}
			for (auto& f : thread_results)
			{
				result += f.get(); // Wait and retrieve each result
			}
			return;
		}
		// else normal simulation
		result=simulation(node->board, node->turn, node->swapped);
		visits=1;
	}

	void selection(Node *& node)
	{
		int k=0;
		//cout<<"in selection children nr:"<<node->children.size()<<endl;
		while (!node->isLeaf() and node->untriedMoves.empty())
		{
			node = node->selectBestUCT();
			k++;
		}
		//cout<<"selection depth: "<<k<<"\n";
	}

	void expension(Node*&node)
	{
		if(!node->untriedMoves.empty() and pos::checkForWinner(node->board)==0)
		{
			//cout<<"created a new child\n";
			int i, j;
			tie(i, j)=node->untriedMoves.back();
			node->untriedMoves.pop_back();
			//cout<<"moves: "<<i<<" "<<j<<endl;

			vector<vector<int>>new_board = node->board;
			bool swapped=(new_board[i][j]!=0)?true :false;
			new_board[i][j]=(turn%2==0)? 1 : -1;

			int new_turn=node->turn+1;

			Node * new_node = new Node(new_board, new_turn, swapped or node->swapped, node);
			new_node->last_move={i,j};
			node->children.push_back(new_node);
			node=new_node;
		}
		else
		{}
			//cout<<"NO EXPENSION -> UNTRIED MOVES EMPTY\n";
	}

	void deleteNodes(Node*&node)
	{
		if(node==NULL)
		{
			//cout<<"NULL!!!!\n";
			return;
		}
		if(node->isLeaf())
		{
			//cout<<"entered isLeaf()\n";
			delete node;
			return;
		}
		
		for(int i=0;i<node->children.size();i++)
		{
			deleteNodes(node->children[i]);
		}
		delete node;
		node=NULL;
	}

	/*Node*findNewRoot()
	{
		Node*null=NULL;
		if(root==NULL)
		{
			return null;
		}
		if(root->board==board)
		{
			return root;
		}
		if(!root->isLeaf())
			for(auto&child : root->children)
			{
				if(child->board==board)
				{
					cout<<"FOUND A NEW ROOT\n";
					return child;
				}
			}
		cout<<"didnt found the new root!!!!!!!!!!!\n";
		return null;
	}*/

}

namespace ai
{
	bool stop_ai=false;
	pair<int, int> move={1, 1};
	state_var state=READY;
	thread t;
	int getIterationLimit()
	{
		if(ai_selection[turn%2]==2)
		{
			switch(int(n))
			{
				case 3: return n*n;
				case 4: return n*n*n*n;
				case 5: return n*n*n*n;
				case 6:
				case 7: return 3*n*n*n;
				case 8: return 3*n*n*n;
				case 9: return n*n*n*n;
			}
		}
		else
		{
			switch(int(n))
			{
				case 3: return n*n*n*n;
				case 4: return n*n*n*n*2;
				case 5: return n*n*n*n*2;
				case 6:
				case 7: return n*n*n*n;
				case 8:
				case 9: return n*n*n*n*3;
			}
		}
		return n*n*n*n;
	}
	void stopAI()
	{
		if(state==WORKING)
			stop_ai=true;
	}
	void iterationThread(vector<vector<int>>board, pair<int,int>&move, int turn)
	{
		mcts::manageRoot();
		//deleteNodes(root);
		//root = new Node(board, turn);
		//cout<<"root swapped:"<<root->swapped<<endl;
		//cout<<"root's board:\n";
		int time_limit;
		time_limit=30000;

		auto start = std::chrono::high_resolution_clock::now();
		int iteration_limit=getIterationLimit();
		//iteration_limit=10000;
		for(int i=0;i<iteration_limit;i++)
		{
			mcts::Node * node = mcts::root;

			//cout<<"\niteration cycle "<<i<<"\n\n";
			//selection
			mcts::selection(node);
			//cout<<"selelction\t";
			//if(node==root)cout<<"ROOT was selected\n";
			//expension
			mcts::expension(node);
			//cout<<"expension\t";
			//if(node==root)cout<<"same as ROOT after expension\n";
			//cout<<"expanded node visits:"<<node->visits<<endl;

			//simulation
			/*int result=simulation(node->board, node->turn);
			cout<<"simulation result="<<result<<endl;*/

			//cout<<"simulation\t";
			int result, visits;
			mcts::runSimulation(node, result, visits);

			//backpropagation
			//backpropagate(node, result);
			//cout<<"backpropagate\t";
			mcts::backpropagate(node, result, visits);

			if(stop_ai)
			{
				break;
			}

			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - start).count();
			if(elapsed>=time_limit)
				break;
		}
		//move=root->bestMove();
		mcts::selectBestMove(move);
		//root=findNewRoot();
		//cout<<"\tvisits\twins\tchildren.size\tuntriedMoves.size\tswap\tlastMove\n";
		//mcts::write_m_children(mcts::root, 2);
		//cout<<"move="<<move.first<<" "<<move.second<<endl;
		//deleteNodes(root);

		state=DONE;
	}
	
	void run()
	{
	
		//check for ai turn
		if(game_stage==game and ai_selection[turn%2]!=0 and pos::checkForWinner(board)==0)
		{
			//cout<<"entered ai:run();\nturn="<<turn<<endl;;
			if(state==READY)
			{
				if(ai_selection[turn%2]==1)
				{
					//to do
					//cout<<"started random move generating\n";
					move=pos::getRandomMove(board, turn);
					state=DONE;
				}

				else
				{
					//cout<<"starttttt"<<endl;
					state=WORKING;
					//t=thread(ai_pair, board, ref(move), ai_selection[turn%2], turn);
					t=thread(iterationThread, board, ref(move), turn);
				}
			}
			if(state==DONE)
			{
				if(t.joinable())
					t.join();
				//cout<<"in done if move:"<<move.first<<"  "<<move.second<<"\n";
				if(stop_ai)
				{
					stop_ai=false;
				}
				else
				{
					makeMove(move.first, move.second);
				}
				//cout<<"enddddddd"<<endl;
				state=READY;
				refreshTextTurn();
			}
		}
	}
}

int main(int argc, char const *argv[])
{
	srand(time(NULL));
	//Font font;
	font.loadFromFile("typo.otf");
	//shapes
	triangle.setColor(Color(0, 0, 0, 255), Color(255, 255, 255, 255));

	border_unit.setFillColor(Color(255, 255,255,255));
	border_unit.setPointCount(3);

	CircleShape hex;
	hex.setPointCount(6);
	hex.setFillColor(Color(211, 211, 211, 255));
	hex.setOutlineColor(Color(0, 0, 0, 255));
	bool click=0;

	resize();
	while (window.isOpen())
	{
		Event event;
		while (window.pollEvent(event))
		{
			if(event.type == Event::Closed)
			{
				mcts::deleteNodes(mcts::root);
				window.close();
			}
			if(event.type == Event::Resized)
			{
				FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window.setView(View(visibleArea));
				resize();
			}
		}
	//update
			ai::run();

        if(Mouse::isButtonPressed(Mouse::Left) and click==0)
		{
			if(game_stage==start_menu)
			{
				if(Bstart.isMouseInside())
				{
					game_stage=game;
					//reset board and turn    (game variables)
					board.assign(board.size(), vector<int>(board.size(), 0));
					turn=0;
					swap_happened=false;
					refreshTextTurn();
					resize();
				}
				if(triangle.isMouseInside()==1 and n<9)
				{
					n++;
					T_TableSize.setString("Table size:"+to_string(int(n)));
				}
				else if(triangle.isMouseInside()==-1 and n>3 )
				{
					n--;
					T_TableSize.setString("Table size:"+to_string(int(n)));
				}
				for(int i=0;i<2;i++)
					for(int j=0;j<4;j++)
					{
						if(ai_button[i][j].isMouseInside())
						{
							ai_selection[i]=static_cast<ai_var>(j);
						}

					}
				//cout<<ai_selection[0]<<endl;
				//cout<<ai_selection[1]<<endl;
				//cout<<endl;
			}
			else //game_stage==game
			{
				//cout<<Mouse::getPosition(window).x<<" "<<Mouse::getPosition(window).y<<endl;
				if(pos::checkForWinner(board)!=0 and con.isMouseInside())
					game_stage=start_menu;
				if(turn==1 and Bswap.isMouseInside())
				{
					swapAct(board);
					turn++;
				}
				if(Bexit.isMouseInside())
				{
					game_stage=start_menu;
					ai::stopAI();
				}
				if(pos::checkForWinner(board)==0 and ai::state==READY)
				{
					float x=(int(n)%2==0)?Mouse::getPosition(window).x : Mouse::getPosition(window).x-hex_rad*sqrt(3)/2, y=Mouse::getPosition(window).y;
					int  b=x/(hex_rad*sqrt(3)/2), c=(x/sqrt(3)+y+hex_rad/2)/hex_rad;
					int a=floor((x/sqrt(3)-y+hex_rad/2)/hex_rad);
					if(c-a>0)
					{
						int i=(c-a-1)/3, j=(b+i)/2;
						j=(int(n)%2==0)? j-n/2 : j-(n-1)/2;
						if(i>=0 and j<n and i<n and j>=0)
							makeMove( i, j);
					}
					refreshTextTurn();
				}
				//cout<<" blue "<<pos::bfs(board, 1)<<endl;
				//cout<<" red "<<pos::bfs(board, -1)<<endl;
				//cout<<" pos ev "<<pos::position_evaluation(board)<<endl;
				//cout<<"simulation="<<mcts::simulation(board, turn, swap_happened)<<endl<<"swap happened:"<<swap_happened<<endl<<"turn="<<turn<<endl;
				//cout<<"pos moves:";
				//write_pos_moves(board, turn);
			}
			click=1;
		}
		if(!Mouse::isButtonPressed(Mouse::Left))
			click=0;

		//draw        
		window.clear();
		if(game_stage==start_menu)
		{
			window.draw(Bstart.hex);
			window.draw(Bstart.text);

			window.draw(T_TableSize.text);
			window.draw(triangle.shape1);
			window.draw(triangle.shape2);

			window.draw(T_PlayerRed.text);
			window.draw(T_PlayerBlue.text);

			for(int i=0; i<2;i++)
				for(int j=0;j<4;j++)
				{
					if(ai_selection[i]==static_cast<ai_var>(j))
						ai_button[i][j].hex.setFillColor(Color(255, 255, 0, 255));
					else
						ai_button[i][j].hex.setFillColor(Color(255, 255, 255, 255));

					window.draw(ai_button[i][j].hex);
					window.draw(ai_button[i][j].text);
				}
		}
		else
		{
			table.draw();
			// draw swap button
			if(turn==1)
			{
				window.draw(Bswap.hex);
				window.draw(Bswap.text);
			}
			//draw texts
			window.draw(T_Turn.text);
			window.draw(T_WhosTurn.text);

			if(pos::checkForWinner(board)!=0)
			{
				window.draw(con.hex);
				window.draw(con.text);
			}
			else
			{
				window.draw(Bexit.hex);
				window.draw(Bexit.text);
			}
		}
		window.display();
	}
	deleteNodes(mcts::root);
	return 0;
}


void resize()
{
	Vector2u w=window.getSize();
	float x, y;
	if( (2*w.x)/(n*3*sqrt(3)+sqrt(3)) <= (2*w.y)/(n*3+1) )
	{
		hex_rad=(2*w.x)/(n*3*sqrt(3)+sqrt(3));
		x=hex_rad*sqrt(3)*( (3*n+1)/2 );
		y=x/sqrt(3);
		unit=y/100;
	}
	else
	{
		hex_rad=(2*w.y)/(3*n+1);
		y=hex_rad*((3*n+1)/2);
		x=y*sqrt(3);
		unit=y/100;
	}

	table.resize();

	Bswap.resize(x-25*unit, y-10*unit, unit*6);
	Bexit.resize(x-25*unit, y-30*unit, unit*6, 1);
	refreshTextTurn();
	T_TableSize.resize(24*unit, y*5/6, 8*unit, "Table size:"+to_string(int(n)));
	triangle.resize(55*unit, y*5/6, 10*unit);

	con.resize(x-25*unit, y-10*unit, 6*unit);
	Bstart.resize(x-25*unit, y*5/6, 6*unit);

	T_PlayerBlue.resize(28*unit, y/6, unit*8);
	T_PlayerRed.resize(28*unit, y*3/6, unit*8);

	ai_button[0][0].resize(T_PlayerBlue.getPosition().x+T_PlayerBlue.text.getGlobalBounds().width/2+20*unit, T_PlayerBlue.getPosition().y, 6*unit, 0.6);
	ai_button[1][0].resize(T_PlayerRed.getPosition().x+T_PlayerRed.text.getGlobalBounds().width/2+20*unit, T_PlayerRed.getPosition().y, 6*unit, 0.6);
	for(int i=0;i<2;i++)
	for(int j=1;j<4;j++)
	{

		ai_button[i][j].resize(ai_button[i][j-1].hex.getPosition().x+ai_button[i][j-1].hex.getLocalBounds().width/2+ai_button[i][j].hex.getGlobalBounds().width/2+5*unit , ai_button[i][j-1].hex.getPosition().y, 6*unit, 0.6);
	}
}

void refreshTextTurn()
{
	T_Turn.resize(20*unit, 25*unit, 8*unit, "Turn nr: "+to_string(turn/2+1));

	if(pos::checkForWinner(board)==1)
	{
		T_WhosTurn.setString("Blue Won!");
		T_WhosTurn.text.setFillColor(blue);
	}
	else if(pos::checkForWinner(board)==-1)
	{
		T_WhosTurn.setString("Red Won!");
		T_WhosTurn.text.setFillColor(Color::Red);
	}
	else if(turn%2==0)
	{
		T_WhosTurn.setString( (ai_selection[turn%2]<2)?"Blue's turn":"Blue thinking");
		T_WhosTurn.text.setFillColor(blue);
	}
	else
	{
		T_WhosTurn.setString( (ai_selection[turn%2]<2)?"Red's turn":"Red thinking");
		T_WhosTurn.text.setFillColor(Color::Red);
	}
	T_WhosTurn.resize(20*unit, 33*unit, 6*unit);
}

void swapAct(vector<vector<int>> &board)
{
	bool br=0;
	for(int i=0;i<n;i++)
	{
		for(int j=0;j<n;j++)
		if(board[i][j]==1)
		{
			board[i][j]=-1;
			br=1;
			break;
		}

		if(br==1)
			break;

	}
}



void makeMove( int i, int j)
{
	if(board[i][j]==0 or turn<=1)
	{
		if(board[i][j]!=0)
			swap_happened=true;
		board[i][j]=turn%2*-2+1;
		turn++;
	}
	//cout<<endl<<"ai_float="<<ai_float(k, board, 4, turn, -INFINITY, INFINITY)<<endl;
	//cout<<"K="<<k<<endl;
	//cout<<position_evaluation(board)<<"-ev"<<endl;
	//cout<<bfs_for_pos_ev(i, j, 1,  board,0).first;
	/*if(pos::checkForWinner(board)==1)
		cout<<"blue won";
	if( pos::checkForWinner(board)==-1)
		cout<<"red won";*/
}



// g++ main.cpp -o app -lsfml-graphics -lsfml-window -lsfml-system && ./app
