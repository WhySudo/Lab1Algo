#include <iostream>
#include <fstream>
#include <algorithm>
#include <cmath>

#define SAVEFILENAME "gameData"

//game params
#define TURN_SCOUNT 10

#define RANDOM_SQUARE_PRICE (17 + rand()%10)
using namespace std;

struct GameValues {
    int currentTurn = 0;
    int population = 100;
    int wheatBunches = 2800;
    int citySpace = 1000;
    int squarePrice = RANDOM_SQUARE_PRICE;

};

struct RoundStats {
    int deadPeople = 0;
    int populationIncome = 0;
    bool plagued = false;
    int collectedWheat = 0;
    int wheatPerSquare = 0;
    int ratEaten = 0;
    bool finished = false;
};

struct UserRoundInput {
    int deltaSquare;
    int bunchesToEat;
    int squareSeed;
};


struct GameState {
    GameValues values;
    RoundStats prevRoundStats;
    float averageDeath = 0;
};


int GetInputWithPrompt(const char *prompt) {
    int result;
    cout << prompt << endl;
    cin >> result;
    return result;
}

void SaveToFile(GameState state) {
    fstream file(SAVEFILENAME, ios_base::out | ios_base::binary | ios_base::trunc);
    if (file.is_open()) {
        cout << sizeof(state) << endl;
        file.write((char *) &state, sizeof(state));
        file.flush();
        file.close();
    }
}

GameState InitGame() {
    GameState state{{},
                    {}};
    bool fileRead = false;
    fstream file(SAVEFILENAME, ios_base::in | ios_base::binary);
    if (file.is_open()) {
        if (file.read((char *) &state, sizeof(GameState))) {
            fileRead = true;
        }
        file.close();
    }
    if (!fileRead) {
        SaveToFile(state);
    }
    return state;

}

bool IsInputValid(UserRoundInput input, GameValues values) {
    int expectedBalance = values.wheatBunches;
    int expectedSquare = values.citySpace;
    if (expectedSquare + input.deltaSquare <= 0) {
        cout << "Мы не можем продать столько земли! " << endl;
        return false;
    }
    int dealWheat = (-input.deltaSquare * values.squarePrice);
    if (expectedBalance + dealWheat < 0) {
        cout << "Мы не можем потратить столько пшеницы! " << endl;
        return false;
    }
    expectedSquare += input.deltaSquare;
    expectedBalance += dealWheat;


    if (input.bunchesToEat > expectedBalance) {
        cout << "Мы не можем потратить столько пшеницы! " << endl;
        return false;
    }

    expectedBalance -= input.bunchesToEat;


    if (input.squareSeed > expectedSquare) {
        cout << "Мы не можем засеять столько пшеницы!" << endl;
        return false;
    }
    if (input.squareSeed > values.population * 10) {
        cout << "У нас мало людей!" << endl;
        return false;
    }
    if (input.squareSeed > expectedBalance * 2) {
        cout << "Мы не можем потратить столько пшеницы! " << endl;
        return false;
    }
    return true;
}

UserRoundInput GetUserRoundInput() {
    UserRoundInput result{};

    result.deltaSquare = GetInputWithPrompt("Сколько акров земли повелеваешь купить? ");
    result.deltaSquare -= GetInputWithPrompt("Сколько акров земли повелеваешь продать? ");
    result.bunchesToEat = GetInputWithPrompt("Сколько бушелей пшеницы повелеваешь съесть? ");
    result.squareSeed = GetInputWithPrompt("Сколько акров земли повелеваешь засеять? ");
    return result;
}

void PrintRoundStats(RoundStats stats, GameValues values) {
    cout << "В " << values.currentTurn << " год правления:" << endl;
    if (stats.deadPeople > 0) {
        cout << "\tПогибло " << stats.deadPeople << " людей" <<endl;
    }
    if (stats.populationIncome > 0) {
        cout << "\tПрибыло " << stats.populationIncome << " людей" << endl;
    }
    if (stats.plagued) {
        cout << "\tСлучилась чума и половина населения погибла" << endl;
    }
    cout << "\tТекущее население: " << values.population << endl;
    cout << "\tСобрано пшеницы: " << stats.collectedWheat << "; Пшеницы с 1 акра: " << stats.wheatPerSquare << endl;
    cout << "\tКрысы съели " << stats.ratEaten << " пшеницы" << endl;
    cout << "\tТекущие запасы пшеницы: " << values.wheatBunches << endl;
    cout << "\tПлощадь города: " << values.citySpace << " акров" << endl;
    cout << "\tЦена за акр в этом году: " << values.squarePrice << endl;

}

RoundStats ProcessTurn(UserRoundInput input, GameValues &game) {
    RoundStats stats;
    game.currentTurn++;
    game.citySpace += input.deltaSquare;
    game.wheatBunches -= input.deltaSquare * game.squarePrice;
    stats.wheatPerSquare = rand() % 5 + 1;
    game.wheatBunches -= (int) ceil(input.squareSeed * 0.5);
    int squaresToCollect = min(input.squareSeed, game.population * 10);
    stats.collectedWheat = squaresToCollect * stats.wheatPerSquare;
    game.wheatBunches += stats.collectedWheat;
    stats.ratEaten = rand() % (int) (0.07 * game.wheatBunches);
    game.wheatBunches -= stats.ratEaten;
    int bunchesToEat = min(game.wheatBunches, input.bunchesToEat);
    game.wheatBunches -= bunchesToEat;
    stats.deadPeople = (game.population - (bunchesToEat / 20));
    if ((float) stats.deadPeople / (float) game.population > 0.45) {
        stats.finished = true;
        return stats;
    }
    game.population -= stats.deadPeople;
    stats.populationIncome = clamp(stats.deadPeople / 2 + ((5 - stats.wheatPerSquare) * game.wheatBunches / 600) + 1, 0,
                                   50);
    game.population += stats.populationIncome;
    stats.plagued = rand() % 100 < 15;
    if (stats.plagued) {
        game.population /= 2;
    }
    return stats;

}

RoundStats GameCycle(GameValues &values) {

    UserRoundInput input;
    bool firstInput = true;
    do {
        if (!firstInput) {
            cout << "Милорд, попробуйте спланировать этот год заново!" << endl;
        }
        firstInput = false;
        input = GetUserRoundInput();
    } while (!IsInputValid(input, values));
    RoundStats stats = ProcessTurn(input, values);
    values.squarePrice = RANDOM_SQUARE_PRICE;
    PrintRoundStats(stats, values);
    return stats;
}

void PrintStats(GameValues values, float averageDeaths) {
    float squarePerCitizen = (float) values.citySpace / (float) values.population;
    if (averageDeaths > 0.33 && squarePerCitizen < 7) {
        cout
                << "Из-за вашей некомпетентности в управлении, народ устроил бунт, и изгнал вас их города. Теперь вы вынуждены влачить жалкое существование в изгнании"
                << endl;
    } else if (averageDeaths > 0.1 && squarePerCitizen < 9) {
        cout
                << "Вы правили железной рукой, подобно Нерону и Ивану Грозному. Народ вздохнул облегчением, и никто больше не желает видеть вас правителем"
                << endl;
    } else if (averageDeaths > 0.33 && squarePerCitizen < 7) {
        cout
                << "Вы справились вполне неплохо, у вас, конечно, есть недоброжелатели, но многие хотели бы увидеть вас во главе города снова"
                << endl;
    } else {
        cout << "Фантастика! Карл Великий, Дизраэли и Джефферсон вместе не справились бы лучше" << endl;
    }
}

int main() {
    setlocale(LC_ALL, "Russian");
    GameState game = InitGame();
    RoundStats stats;
    bool lostDead = false;
    if (game.values.currentTurn > 0) {
        PrintRoundStats(game.prevRoundStats, game.values);
    }
    for (int i = game.values.currentTurn; i < TURN_SCOUNT; ++i) {
        stats = GameCycle(game.values);
        if (stats.finished) {
            lostDead = true;
            break;
        }
        game.prevRoundStats = stats;
        game.averageDeath += ((float) stats.deadPeople / (float) (game.values.population + stats.deadPeople));

        SaveToFile(game);
    }
    if (!lostDead) {
        game.averageDeath /= (float) TURN_SCOUNT;
        PrintStats(game.values, game.averageDeath);
    }
    SaveToFile(GameState{});
    return 0;

}


