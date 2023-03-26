#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
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

vector<Place> get_command_line(int argc, char const* argv[])
{
	ifstream instream;
	instream.open(argv[1]);
	if (instream.fail())
	{
		cout << "Input file opening failed.\n";
		exit(1);
	}
	vector<string> lines;
	string temp;
	while (getline(instream, temp))
		lines.push_back(temp);
	instream.close();
	vector<string> title = seperate_words(lines[0]);
	vector<Place> Places;
	int name_index = distance(title.begin(), find(title.begin(), title.end(), "name"));
	int rank_index = distance(title.begin(), find(title.begin(), title.end(), "rank"));
	int opentime_index = distance(title.begin(), find(title.begin(), title.end(), "openingTime"));
	int closetime_index = distance(title.begin(), find(title.begin(), title.end(), "closingTime"));
	for (int i = 1; i < lines.size(); i++) {
		Place temp;
		vector<string> words_in_line = seperate_words(lines[i]);
		temp.name = words_in_line[name_index];
		temp.rank = stoi(words_in_line[rank_index]);
		temp.open_time.hour = stoi(words_in_line[opentime_index]);
		temp.open_time.minute = stoi(words_in_line[opentime_index].substr(3));
		temp.close_time.hour = stoi(words_in_line[closetime_index]);
		temp.close_time.minute = stoi(words_in_line[closetime_index].substr(3));
		temp.have_gone = false;
		Places.push_back(temp);
	}
	sort(Places.begin(), Places.end(), compare_rank);
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

Time skip_time(const vector<Place> Places, Time now)
{
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
	vector<Place> Places = get_command_line(argc, argv);
	where_to_go(Places);
	return 0;
}
