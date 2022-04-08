#include "json.h"

#include <cstdio>
#include <stdexcept>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;
    char c;
    for (; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c!=']') {
    	throw ParsingError("Array not closed with ] symbol");
    }

    return Node(move(result));
}

Node LoadNum(istream& input) {
    string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(istream& input) {
	string lineresult;
    char c;
    bool prev_bckslsh = false;
    bool prev_bckslsh_nreset = true;
    c = input.get();
    for (; !input.eof();) {
    	if ((c == '\"') && (!prev_bckslsh)) {
    		break;
    	}
    	prev_bckslsh_nreset = true;

    	if ((!prev_bckslsh) && (c != '\\')) {
    		lineresult += c;
    	}

    	if (prev_bckslsh) {
    		if (c == 't') {
    			//cout << "Found tab" << endl;
    			lineresult += '\t';
    		}
    		if (c == 'n') {
    			//cout << "Found new string symbol" << endl;
    			lineresult += '\n';
    		}
    		if (c == 'r') {
    			//cout << "Found \\r symbol" << endl;
    			lineresult += '\r';
    		}
    		if (c == '\"') {
    			//cout << "Found quotes" << endl;
    			lineresult += '\"';
    		}
    		if (c == '\\') {
    			//cout << "Found backslash" << endl;
    			lineresult += '\\';
    			prev_bckslsh_nreset = false;
    		}
    	}
    	prev_bckslsh = ((c == '\\') && prev_bckslsh_nreset);
    	c = input.get();
    }

    if (!((c == '\"') && (!prev_bckslsh))) {
    	throw ParsingError("Something terribly wrong with closing quotes in string");
    }
    return Node(move(lineresult));
}

Node LoadDict(istream& input) {
    Dict result;
    //cout << "Processing map" << endl;
    char c;
    for (; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        //cout << key << endl;
        input >> c;
        //cout << c << endl;
        Node value = LoadNode(input);
        //Print(Document{value}, cout);
        //cout << endl;
        result.insert({move(key), value});
    }

    if (c != '}') {
    	throw ParsingError("Map not closed with } symbol");
    }

    //cout << "Done processing map" << endl;
    return Node(move(result));
}

Node LoadBool(istream& input) {
	//cout << "Processing bool" << endl;
	/*
	string test;
	input >> test;
	cout << test << endl;
	bool result;
	if (test == "true") {
		result = true;
	} else if (test == "false") {
		result = false;
	} else {
		throw ParsingError("In input stream found illegal word which starts with t or f letter");
	}
	*/
	char c;
	string test;
	bool result;
	input >> c;
	if (c == 't'){
		test += c;
		for (int i=0; ((i<3) && (!input.eof())); i++){
			input >> c;
			test += c;
		}
		//cout << test << endl;
		if (test == "true") {
			result = true;
		} else {
			throw ParsingError("Unknown keyword in JSON stream");
		}
	} else if (c == 'f') {
		test += c;
		for (int i=0; ((i<4) && (!input.eof())); i++) {
			input >> c;
			test += c;
		}
		//cout << test << endl;
		if (test == "false") {
			result = false;
		} else {
			throw ParsingError("Unknown keyword in JSON stream");
		}
	}
	//cout << "Done processing bool" << endl << endl;
	return Node(result);
}

Node LoadNull(istream& input) {
	//string test;
	//input >> test;
	char c;
	string test;
	for (int i=0; (i<4) && (!input.eof()); i++){
		c = input.get();
		test += c;
	}
	if (test == "null") {
		return Node(nullptr);
	} else {
		throw ParsingError("Unknown keyword in JSON stream");
	}
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    //cout << c << endl;
    if (c == '[') {
    	//cout << "Arr found" << endl;
        return LoadArray(input);
    } else if (c == '{') {
    	//cout << "Map found" << endl;
    	return LoadDict(input);
    } else if (c == '\"') {
    	//cout << "String found" << endl;
        return LoadString(input);
    } else if (c == 'n') {
    	//cout << "Null found" << endl;
    	input.putback(c);
    	return LoadNull(input);
    } else if ((c == 't') || (c == 'f')) {
    	//cout << "Logical found" << endl;
    	input.putback(c);
    	return LoadBool(input);
	} else {
		//cout << "Number found" << endl;
        input.putback(c);
        return LoadNum(input);
    }
}

}  // namespace



Node::Node(Array array)
    : value_(move(array)) {
}

Node::Node(Dict map)
    : value_(move(map)) {
}

Node::Node(int value)
    : value_(value) {
}

Node::Node(double value)
    : value_(value) {
}

Node::Node(string value)
    : value_(move(value)) {
}

Node::Node(nullptr_t): value_(nullptr){
}

Node::Node(bool value): value_(value){
}


const Array& Node::AsArray() const {
	if (!IsArray()){
		throw logic_error("Conversion from non-array to array");
	}
	return get<Array>(value_);
}

const Dict& Node::AsMap() const {
	if (!IsMap()){
		throw logic_error("Conversion from non-dict to dict");
	}
    return get<Dict>(value_);
}

int Node::AsInt() const {
	if (!IsInt()){
		throw logic_error("Conversion from non-int to int");
	}
    return get<int>(value_);
}

double Node::AsDouble() const {
	if (IsPureDouble()) {
		return get<double>(value_);
	} else if (IsInt()) {
		return static_cast<double>(get<int>(value_));
	} else {
		throw logic_error("Conversion non-number to double");
	}
}

const string& Node::AsString() const {
	if (!IsString()){
		throw logic_error("Conversion from non-string to string");
	}
    return get<string>(value_);
}

bool Node::AsBool() const {
	if (!IsBool()){
		throw logic_error("Conversion from non-bool to bool");
	}
    return get<bool>(value_);
}

bool Node::IsInt() const {
	return holds_alternative<int>(value_);
}

bool Node::IsDouble() const {
	return ((holds_alternative<int>(value_)) || (holds_alternative<double>(value_)));
}

bool Node::IsPureDouble() const {
	return holds_alternative<double>(value_);
}

bool Node::IsBool() const {
	return holds_alternative<bool>(value_);
}

bool Node::IsString() const {
	return holds_alternative<string>(value_);
}

bool Node::IsArray() const {
	return holds_alternative<Array>(value_);
}

bool Node::IsMap() const {
	return holds_alternative<Dict>(value_);
}

bool Node::IsNull() const {
	return holds_alternative<nullptr_t>(value_);
}

Document::Document(Node root)
    : root_(move(root)) {
}


const Node& Document::GetRoot() const {
    return root_;
}

/*
Node& Document::GetRoot() const {
    return root_;
}
*/

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void PrintSpaces(size_t delim_num, std::ostream& output) {
	for (int i=0; i < delim_num; i++) {
		output << " ";
	}
}

void PrintWithDelims(const Document& doc, std::ostream& output, size_t delim_num, bool print_delims) {
	Node to_print = doc.GetRoot();
	if (to_print.IsPureDouble()) {
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << to_print.AsDouble();
	} else if (to_print.IsInt()) {
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << to_print.AsInt();
	} else if (to_print.IsNull()) {
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << "null"s;
	} else if (to_print.IsString()) {
		string str = to_print.AsString();
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << "\""s;
		for (char letter: str) {
			if (letter == '\\') {
				output << "\\\\"s;
			} else if (letter == '\t') {
				output << "\\t"s;
			} else if (letter == '\n') {
				output << "\\n"s;
			} else if (letter == '\r') {
				output << "\\r"s;
			} else if (letter == '\"') {
				output << "\\\"";
			} else {
				output << letter;
			}
		}
		output << "\""s;
	} else if (to_print.IsBool()) {
		if (print_delims) {PrintSpaces(delim_num, output);}
		if (to_print.AsBool()) {
			output << "true";
		} else {
			output << "false";
		}
	} else if (to_print.IsArray()) {
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << "[\n";
		bool start = true;
		for(const Node& node: to_print.AsArray()){
			if (start) {
				start = !start;
			} else {
				output << ",\n";
			}
			PrintWithDelims(Document{node}, output, delim_num + 4, true);
		}
		output << "\n";
		PrintSpaces(delim_num, output);
		output << "]";
	} else if (to_print.IsMap()) {
		bool start = true;
		if (print_delims) {PrintSpaces(delim_num, output);}
		output << "{\n";
		for(auto& [key, value]: to_print.AsMap()) {
			if (start) {
				start = !start;
			} else {
				output << ",\n";
			}
			if (print_delims) {PrintSpaces(delim_num + 4, output);}
			output << "\""s;
			output << key;
			output << "\""s;
			output << ": ";
			PrintWithDelims(Document{value}, output, delim_num + 4, false);
		}
		output << "\n";
		PrintSpaces(delim_num, output);
		output << "}";
	}
}

void Print(const Document& doc, std::ostream& output) {
	PrintWithDelims(doc, output, 0, true);
}

bool operator== (const Node& lhs, const Node& rhs) {
	bool result = false;
	result = result || ((lhs.IsArray()) && (rhs.IsArray()) && (lhs.AsArray() == rhs.AsArray()));
	result = result || ((lhs.IsMap()) && (rhs.IsMap()) && (lhs.AsMap() == rhs.AsMap()));
	result = result || ((lhs.IsNull()) && (rhs.IsNull()));
	result = result || ((lhs.IsString()) && (rhs.IsString()) && (lhs.AsString() == rhs.AsString()));
	result = result || ((lhs.IsInt()) && (rhs.IsInt()) && (lhs.AsInt() == rhs.AsInt()));
	result = result || ((lhs.IsPureDouble()) && (rhs.IsPureDouble()) && (lhs.AsDouble() == rhs.AsDouble()));
	result = result || ((lhs.IsBool()) && (rhs.IsBool()) && (lhs.AsBool() == rhs.AsBool()));
	return result;
}

bool operator!= (const Node& lhs, const Node& rhs) {
	return !(lhs == rhs);
}

bool operator== (const Document& lhs, const Document& rhs) {
	return (lhs.GetRoot() == rhs.GetRoot());
}

bool operator!= (const Document& lhs, const Document& rhs) {
	return !(lhs == rhs);
}

}  // namespace json
