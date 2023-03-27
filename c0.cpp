#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>

#define CLOCK_MINUTE_THRESHOLD 3
#define INPUT_ERROR_MSG "Input file opening failed."
#define NAME_COLUMN "name"
#define RANK_COLUMN "rank"
#define OPENING_TIME_COLUMN "openingTime"
#define CLOSING_TIME_COLUMN "closingTime"
using namespace std;

struct Time {
	int hour;
	int minute;
};

struct Place {
	string name;
	int rank;
	Time open_time;
	Time close_time;
	bool have_gone;
};

void throw_error(string msg) {
	cout << msg << endl;
	exit(1);
}

bool compare_rank(const Place &i, const Place &j) {
	return i.rank < j.rank;
}

vector<string> seperate_words(const string line, string separate_char = ",") {
	vector<string> words;
	string temp = line;
	size_t pos = 0;
	while ((pos = temp.find(separate_char)) != string::npos) {
		words.push_back(temp.substr(0, pos));
		temp.erase(0, pos + separate_char.length());
	}
	words.push_back(temp);
	return words;
}

Time create_time(string time_string) {
	Time temp;
	temp.hour = stoi(time_string);
	temp.minute = stoi(time_string.substr(CLOCK_MINUTE_THRESHOLD));
	return temp;
}

Place create_place(string name, int rank, Time opentime, Time closetime, bool have_gone = false) {
	Place temp;
	temp.name = name;
	temp.rank = rank;
	temp.open_time = opentime;
	temp.close_time = closetime;
	temp.have_gone = have_gone;
	return temp;
}

vector<string> get_input_lines(int argc, char const* argv[]) {
	ifstream instream;
	instream.open(argv[1]);
	if (instream.fail()) {
		throw_error(INPUT_ERROR_MSG);
	}
	vector<string> lines;
	string temp;
	while (getline(instream, temp)) {
		lines.push_back(temp);
	}
	instream.close();
	return lines;
}

vector<Place> get_places(const vector<string> &lines) {
	vector<string> title = seperate_words(lines[0]);
	vector<Place> Places;
	int name_index = distance(title.begin(), find(title.begin(), title.end(), NAME_COLUMN));
	int rank_index = distance(title.begin(), find(title.begin(), title.end(), RANK_COLUMN));
	int opentime_index = distance(title.begin(), find(title.begin(), title.end(), OPENING_TIME_COLUMN));
	int closetime_index = distance(title.begin(), find(title.begin(), title.end(), CLOSING_TIME_COLUMN));
	for (int i = 1; i < lines.size(); i++) {
		Place temp;
		vector<string> words_in_line = seperate_words(lines[i]);
		temp = create_place(words_in_line[name_index], stoi(words_in_line[rank_index]), create_time(words_in_line[opentime_index]), create_time(words_in_line[closetime_index]));
		Places.push_back(temp);
	}
	return Places;
}

int during_time(Place plc, Time now) {
	return (plc.close_time.hour - now.hour) * 60 + (plc.close_time.minute - now.minute);
}

Time add_time(Time now, int during_time) {
	if (during_time >= 60) {
		now.hour += 1;
		return now;
	}
	now.minute += during_time;
	if (now.minute >= 60) {
		now.hour++;
		now.minute -= 60;
	}
	return now;
}

bool is_time_greater(Time t1, Time t2) {
	if (t1.hour * 60 + t1.minute >= t2.hour * 60 + t2.minute)
		return true;
	return false;
}

int find_first_open_time(const vector<Place> Places, Time start_time = { 0,0 })
{
	int index = -1;
	int i;
	for (i = 0; i < Places.size(); i++)
		if (is_time_greater(Places[i].open_time, start_time)) {
			index = i;
			break;
		}
	for (int j = i + 1; j < Places.size(); j++) {
		if (Places[j].open_time.hour < Places[index].open_time.hour && is_time_greater(Places[j].open_time, start_time)) {
			index = j;
			continue;
		}
		if (Places[j].open_time.hour == Places[index].open_time.hour && Places[j].open_time.minute < Places[index].open_time.minute && is_time_greater(Places[j].open_time, start_time))
			index = j;
	}
	return index;
}

int find_best_place_index(const vector<Place> Places, const Time now) {
	int index = -1;
	if (now.minute == -1) {
		index = find_first_open_time(Places);
	}
	else {
		for ( int i = 0; i < Places.size(); i++) {
			if (!Places[i].have_gone && during_time(Places[i], now) >= 15 && is_time_greater(now, Places[i].open_time)) {
				index = i;
				return index;
			}
		}
	}
	return index;
}

string time_to_string(Time t) {
	string str;
	if (t.hour < 10)
		str += "0" + to_string(t.hour);
	else
		str += to_string(t.hour);
	str += ":";
	if (t.minute < 10)
		str += "0" + to_string(t.minute);
	else
		str += to_string(t.minute);
	return str;
}

void print_place(Place plc, Time start_time, Time end_time) {
	string start = time_to_string(start_time);
	string end = time_to_string(end_time);
	cout << "Location " << plc.name << endl;
	cout << "Visit from " << start << " until " << end << endl;
	cout << "---" << endl;
}

Time skip_time(const vector<Place> Places, Time now) {
	int index = find_first_open_time(Places, now);
	Time skiped_time = { -1,-1 };
	if ( index == -1)
		return skiped_time;
	else {
		skiped_time = Places[index].open_time;
		return skiped_time;
	}
}

void where_to_go(vector<Place> Places) {
	Time now = { -1,-1 };
	Time end_time;
	for (int i = 0; i < Places.size(); i++) {
		int index = find_best_place_index(Places, now);
		if ( index == -1) {
			now = skip_time(Places, add_time(now, 1));
			if (now.hour == -1)
				break;
			else {
				i = i - 1;
				continue;
			}
		}
		if ( now.minute == -1) {
			now.hour = Places[index].open_time.hour;
			now.minute = Places[index].open_time.minute;
			end_time = add_time(now, during_time(Places[index], now));
		}
		else
			end_time = add_time(now, during_time(Places[index], now));
		Places[index].have_gone = true;
		print_place(Places[index], now, end_time);
		now = add_time(end_time, 30);
	}
}

int main(int argc, char const* argv[])
{
	vector<Place> Places = get_places(get_input_lines(argc, argv));
	sort(Places.begin(), Places.end(), compare_rank);
	where_to_go(Places);
	return 0;
}
