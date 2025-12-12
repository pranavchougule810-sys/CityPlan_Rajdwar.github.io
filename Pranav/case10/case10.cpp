#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <iomanip>
using namespace std;

struct Player {
    string playerID;
    string name;
    int score;
    int tokensUsed;
    string favoriteGame;
    
    bool operator<(const Player& other) const {
        return score < other.score; // Max heap for leaderboard
    }
};

struct GameCard {
    string cardID;
    int tokens;
    double amountPaid;
};

class GamingZone {
private:
    unordered_map<string, GameCard> cards;
    unordered_map<string, int> gameTokenCost;
    priority_queue<Player> leaderboard; // Max heap
    vector<Player> allPlayers;
    double totalRevenue;
    
public:
    GamingZone() : totalRevenue(0.0) {
        initializeGames();
    }
    
    void initializeGames() {
        gameTokenCost["VR_Racing"] = 15;
        gameTokenCost["Arcade_Shooter"] = 5;
        gameTokenCost["Bowling"] = 10;
        gameTokenCost["Air_Hockey"] = 8;
        gameTokenCost["VR_Adventure"] = 20;
    }
    
    string buyTokens(string cardID, int tokens) {
        GameCard card;
        card.cardID = cardID;
        card.tokens = tokens;
        card.amountPaid = tokens * 10.0;
        
        if(tokens > 50) card.amountPaid *= 0.9; // 10% discount
        if(tokens > 100) card.amountPaid *= 0.85; // 15% discount
        
        cards[cardID] = card;
        totalRevenue += card.amountPaid;
        
        return cardID;
    }
    
    bool playGame(string cardID, string game, string playerID, string playerName) {
        if(cards.find(cardID) != cards.end() && gameTokenCost.find(game) != gameTokenCost.end()) {
            GameCard& card = cards[cardID];
            int cost = gameTokenCost[game];
            
            if(card.tokens >= cost) {
                card.tokens -= cost;
                
                // Update player stats
                bool found = false;
                for(auto& player : allPlayers) {
                    if(player.playerID == playerID) {
                        player.score += (cost * 10); // Score based on game
                        player.tokensUsed += cost;
                        found = true;
                        break;
                    }
                }
                
                if(!found) {
                    Player newPlayer;
                    newPlayer.playerID = playerID;
                    newPlayer.name = playerName;
                    newPlayer.score = cost * 10;
                    newPlayer.tokensUsed = cost;
                    newPlayer.favoriteGame = game;
                    allPlayers.push_back(newPlayer);
                }
                
                return true;
            }
        }
        return false;
    }
    
    void buildLeaderboard() {
        leaderboard = priority_queue<Player>(); // Clear
        for(auto& player : allPlayers) {
            leaderboard.push(player);
        }
    }
    
    void displayLeaderboard(int top) {
        cout << "\n========== Top " << top << " Players Leaderboard ==========\n";
        priority_queue<Player> temp = leaderboard;
        int rank = 1;
        
        while(!temp.empty() && rank <= top) {
            Player p = temp.top();
            temp.pop();
            cout << rank << ". " << p.name << " | Score: " << p.score
                 << " | Tokens: " << p.tokensUsed << endl;
            rank++;
        }
    }
    
    void displayStatistics() {
        cout << "\n========== Gaming Zone Statistics ==========\n";
        cout << "Total Cards Issued: " << cards.size() << endl;
        cout << "Total Players: " << allPlayers.size() << endl;
        cout << "Total Revenue: Rs." << fixed << setprecision(2) << totalRevenue << endl;
    }
    
    void loadFromCSV(string filename) {
        ifstream file(filename);
        if(!file.is_open()) {
            cout << "Error: Could not open " << filename << endl;
            return;
        }
        
        string line;
        getline(file, line); // Skip header
        
        while(getline(file, line)) {
            stringstream ss(line);
            string cardID, playerID, playerName, game;
            int tokens;
            
            getline(ss, cardID, ',');
            getline(ss, playerID, ',');
            getline(ss, playerName, ',');
            ss >> tokens;
            ss.ignore();
            getline(ss, game, ',');
            
            buyTokens(cardID, tokens);
            playGame(cardID, game, playerID, playerName);
        }
        
        file.close();
        cout << "Data loaded successfully from " << filename << endl;
    }
};

int main() {
    GamingZone zone;
    
    cout << "========== Entertainment & Gaming Zone System ==========\n";
    cout << "Loading data from case10.csv...\n";
    
    zone.loadFromCSV("case10.csv");
    zone.displayStatistics();
    zone.buildLeaderboard();
    zone.displayLeaderboard(15);
    
    return 0;
}
