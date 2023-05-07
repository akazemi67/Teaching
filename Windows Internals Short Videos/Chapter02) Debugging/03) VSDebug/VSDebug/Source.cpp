#include "json.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>

//https://github.com/nlohmann/json
using json = nlohmann::json;
using namespace std;

int before_christ_books;

struct Book {
	string author, title, language, country;
	int pages, year;
};

Book parse_book(const json &jsonObj) {
	Book book;
	book.author = jsonObj["author"];
	book.title = jsonObj["title"];
	book.language = jsonObj["language"];
	book.country = jsonObj["country"];
	book.pages = jsonObj["pages"];
	book.year = jsonObj["year"];
	if (book.year < 0)
		before_christ_books++;
	return book;
}

vector<Book>& parse_all_books(const json &jsonArray) {
	const int n = jsonArray.size();
	auto *books = new vector<Book>(n);
	for(auto i=0; i<n; ++i) {
		(*books)[i] = parse_book(jsonArray[i]);
	}
	return *books;
}

void print_header() {
	cout << "Title " << setw(50) << "| Author" << setw(44) << "| Year" << endl;
	for (auto i = 0; i < 100; i++)
		cout << "-";
	cout << endl;
}

void print_tablebook(const Book& book) {
	int len = book.title.length();
	cout << book.title << setw(50 - len);
	len = book.author.length() + 2;
	cout << "| " << book.author << setw(50 - len);
	cout << book.year << endl;
}

void clear_screen() {
	//cout << "\033[2J\033[1;1H";
	system("cls");
}

string title(const Book& book) {
	return "Title[" + book.title + "] ";
}

string author(const Book& book) {
	return "Author[" + book.author + "] ";
}

string language(const Book& book) {
	return "Lang[" + book.language + "] ";
}

string year(const Book& book) {
	return "Year[" + to_string(book.year) + "]";
}

const Book& get_book(const vector<Book>& books, int index) {
	if (!(index > 0 && index <= 50))
		index = 0;
	return books[index];
}

string get_book_info(const vector<Book>& books, int index) {
	const Book& book = get_book(books, index);
	return title(book) + author(book) + language(book) + year(book);
}

int main() {
	ifstream booksFile("books.json");
	
	string sampleInput = "[\n"
		"  {\n"
		"    \"author\": \"Chinua Achebe\",\n"
		"    \"country\": \"Nigeria\",\n"
		"    \"imageLink\": \"images/things-fall-apart.jpg\",\n"
		"    \"language\": \"English\",\n"
		"    \"link\": \"https://en.wikipedia.org/wiki/Things_Fall_Apart\\n\",\n"
		"    \"pages\": 209,\n"
		"    \"title\": \"Things Fall Apart\",\n"
		"    \"year\": 1958\n"
		"  },\n"
		"  {\n"
		"    \"author\": \"Hans Christian Andersen\",\n"
		"    \"country\": \"Denmark\",\n"
		"    \"imageLink\": \"images/fairy-tales.jpg\",\n"
		"    \"language\": \"Danish\",\n"
		"    \"link\": \"https://en.wikipedia.org/wiki/Fairy_Tales_Told_for_Children._First_Collection.\\n\",\n"
		"    \"pages\": 784,\n"
		"    \"title\": \"Fairy tales\",\n"
		"    \"year\": 1836\n"
		"  }\n"
		"]";
	
	if(!booksFile.is_open()) {
		cout << "Current Path: " << filesystem::current_path() << endl;
		cout << "Error opening books file." << endl;
		return 1;
	}
	json data = json::parse(booksFile);
	vector<Book> books = parse_all_books(data);
	
	int choice;
	do {
		clear_screen();
		cout << "1) Print all books." << endl;
		cout << "2) Print book info." << endl;
		cout << "3) Number of books from B.C." << endl;
		cout << "ELSE) Exit" << endl;
		cout << "Selection: ";
		cin >> choice;

		switch (choice) {
		case 1:
			print_header();
			for (auto& b : books) {
				print_tablebook(b);
			}
			break;

		case 2:
			int book_number;
			cout << "Type book number: ";
			cin >> book_number;
			cout << "Book Info: " << endl;
			cout << get_book_info(books, book_number - 1);
			break;

		case 3:
			cout << "\n#of Books from B.C: " << before_christ_books << endl;
			break;

		default:
			exit(0);
		}

		cout << "\n\n\tPress any key to continue...." << endl;
		cin.get();
		cin.get();
	} while (true);
	
	return 0;
}

