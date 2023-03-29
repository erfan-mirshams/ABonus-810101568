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
#define MINUTES_IN_HOUR 60
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

vector<string> seperate_words(const string line, string delim = ",") {
	vector<string> words;
	string temp = line;
	size_t pos = 0;
	while ((pos = temp.find(delim)) != string::npos) {
		words.push_back(temp.substr(0, temp.find(delim)));
		temp.erase(0, pos + delim.length());
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
	vector<Place> places;
	int name_index = distance(title.begin(), find(title.begin(), title.end(), NAME_COLUMN));
	int rank_index = distance(title.begin(), find(title.begin(), title.end(), RANK_COLUMN));
	int opentime_index = distance(title.begin(), find(title.begin(), title.end(), OPENING_TIME_COLUMN));
	int closetime_index = distance(title.begin(), find(title.begin(), title.end(), CLOSING_TIME_COLUMN));
	for (int i = 1; i < lines.size(); i++) {
		Place temp;
		vector<string> words_in_line = seperate_words(lines[i]);
		temp = create_place(words_in_line[name_index], stoi(words_in_line[rank_index]),
							create_time(words_in_line[opentime_index]), create_time(words_in_line[closetime_index]));
		places.push_back(temp);
	}
	return places;
}

int during_time(Place plc, Time now) {
	return (plc.close_time.hour - now.hour) * MINUTES_IN_HOUR + (plc.close_time.minute - now.minute);
}

Time add_time(Time now, int during_time) {
	if (during_time >= MINUTES_IN_HOUR) {
		now.hour++;
		return now;
	}
	now.minute += during_time;
	if (now.minute >= MINUTES_IN_HOUR) {
		now.hour++;
		now.minute -= MINUTES_IN_HOUR;
	}
	return now;
}

bool is_time_greater(Time t1, Time t2) {
	return (t1.hour * MINUTES_IN_HOUR + t1.minute >= t2.hour * MINUTES_IN_HOUR + t2.minute);
}

int find_first_open_time(const vector<Place> places, Time start_time = {0, 0}) {
	int index = -1;
	for (int i = 0; i < places.size(); i++) {
		if (is_time_greater(places[i].open_time, start_time)) {
			if(index == -1 || (is_time_greater(places[index].open_time, places[i].open_time))) {
				index = i;
			}
		}
	}
	return index;
}

int find_best_place_index(const vector<Place> places, const Time now) {
	int index = -1;
	if (now.minute == -1) {
		index = find_first_open_time(places);
		return index;
	}
	for (int i = 0; i < places.size(); i++) {
			if (!places[i].have_gone && during_time(places[i], now) >= (MINUTES_IN_HOUR / 4) && is_time_greater(now, places[i].open_time)) {
			index = i;
			return index;
		}
	}
	return index;
}

string time_to_string(Time t) {
	string str = "";
	str += (t.hour < 10 ? "0" : "") + to_string(t.hour) + ":" + (t.minute < 10 ? "0" : "") + to_string(t.minute);
	return str;
}

string print_place_value(Place plc, Time start_time, Time end_time) {
	string val = "";
	string start = time_to_string(start_time);
	string end = time_to_string(end_time);
	val += "Location" + plc.name + "\n";
	val += "Visit from " + start + " until" + end + "\n";
	val += "---\n";
	return val;
}

Time skip_time(const vector<Place> places, Time now) {
	int index = find_first_open_time(places, now);
	Time skipped_time = {-1, -1};
	if (index == -1) {
		return skipped_time;
	}
	else {
		skipped_time = places[index].open_time;
		return skipped_time;
	}
}

void where_to_go(vector<Place> places) {
	Time now = {-1, -1};
	Time end_time;
	for (int i = 0; i < places.size(); i++) {
		int index = find_best_place_index(places, now);
		if (index == -1) {
			now = skip_time(places, add_time(now, 1));
			if (now.hour == -1) {
				break;
			}
			index = find_best_place_index(places, now);
		}
		if (now.minute == -1) {
			now.hour = places[index].open_time.hour;
			now.minute = places[index].open_time.minute;
		}
		end_time = add_time(now, during_time(places[index], now));
		places[index].have_gone = true;
		cout << print_place_value(places[index], now, end_time);
		now = add_time(end_time, MINUTES_IN_HOUR / 2);
	}
}

int main(int argc, char const* argv[])
{
	vector<Place> places = get_places(get_input_lines(argc, argv));
	sort(places.begin(), places.end(), compare_rank);
	where_to_go(places);
	return 0;
}
