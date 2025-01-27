#include "db_tools.hpp"
#include "sqlite3.h"
#include <string>
#include <queue>
#include <regex>
#include <boost/filesystem.hpp>
#include <iostream>

namespace fs = boost::filesystem;

/**
 * Constructor opens the database
 */
Db::Db(std::string db_path) {
    // Create database connection
    sqlite3* dbc;
    int ec;

    if(db_path.empty()){
        // Create table in memory
        ec = sqlite3_open(NULL, &dbc);
    }
    else if(fs::path(db_path).has_extension()){
        ec = sqlite3_open(db_path.c_str(), &dbc);
    }
    else{
        std::cout << "db path needs to have extension " << std::endl;
        return;
    }

    // Return if error opening database
    if (ec != SQLITE_OK) {
        std::cout << "Could not open database: " << sqlite3_errmsg(dbc) << std::endl;
        return;
    }

    // Store database connection
    this->dbc = dbc;

    // Create file tags table if not already present
    this->CreateFileTagsTable();
}

/**
 * Create file_tags table
 */
void Db::CreateFileTagsTable() {

    // Create sql statement
    std::string sql =
        "create table if not exists file_tags ("
        "path text,"
        "tag text);";

    this->ExecSqlNoCallback(sql);
}

/**
 * Execute SQL for the open database without a callback
 */
void Db::ExecSqlNoCallback(std::string sql) {
    // Execute sql
    char* err_msg = 0;
    int ec;
    ec = sqlite3_exec(this->dbc, sql.c_str(), NULL, NULL, &err_msg);
    if (ec != SQLITE_OK) {
        std::cout << "Error executing sql:\n\t" << err_msg << std::endl;
        sqlite3_free(err_msg);
        return;
    }
}

/**
 * Destructor closes the database
 */
Db::~Db() {
    // Close database
    if(this->dbc){
        sqlite3_close(this->dbc);
    }
}

/**
 * Check if database is open
 */
bool Db::IsOpen(){
    if(this->dbc){
        return true;
    }
    else{
        return false;
    }
}

/**
 * Insert a row into the table of the database
 */
void Db::InsertRow(std::string path, std::string tag) {

    // Convert to absolute path and convert forward slashes
    std::string abs_path = fs::canonical(path).make_preferred().string();

    // Escape backslashes before running sql
    std::string abs_path_esc = "";
    for(auto &ch: abs_path){
        if(ch != '\\'){
            abs_path_esc += ch;
        }
        else{
            abs_path_esc += "\\\\";
        }
    }

    // Create sql statement
    std::string sql = "insert into file_tags values(\"" + abs_path_esc + "\",\"" + tag + "\");";

    // Execute sql
    this->ExecSqlNoCallback(sql);
}

/**
 * Select paths from the table of the database
 */
std::vector<std::string> Db::SelectTagQuery(std::string tag_query) {

    std::vector<std::string> result_paths;

    // Generate sql statement
    std::string sql;
    try{
        sql = this->GenSql(tag_query);
    }
    catch(const std::invalid_argument& e){
        std::cout << "GenSql Exception: \n\t" << e.what() << std::endl;
    }

    // Display the sql statement
    // std::cout << sql << std::endl;

    // Execute sql
    char* err_msg = 0;
    int ec;
    ec = sqlite3_exec(this->dbc, sql.c_str(), this->SelectCallback, static_cast<void*>(&result_paths), &err_msg);
    if (ec != SQLITE_OK) {
        std::cout << "Error executing select statement:\n\t" << err_msg << std::endl;
        sqlite3_free(err_msg);
        return result_paths;
    }

    return result_paths;
}

/**
 * Callback for select statement
 */
int Db::SelectCallback(void* data, int n_cols, char** values, char** headers) {

    std::vector<std::string>* result_paths = static_cast<std::vector<std::string>*>(data);

    // Store result
    result_paths->push_back(values[0]);

    return 0;
}

/**
 * Tokenize an input query
 */
std::vector<std::string> Db::Tokenize(std::string query) {

    std::vector<std::string> result;

    // Regex expression
    std::regex ex("\\w+");

    std::regex_iterator<std::string::iterator> pos(query.begin(),query.end(), ex);
    std::regex_iterator<std::string::iterator> end; // Default constructor defines past-the-end iterator
    for (; pos != end; pos++) {
        result.push_back(pos->str(0));
    }

    return result;
}

/**
 * Check if token is a logical
 */
bool Db::IsLogical(std::string token){
    if(token == "not" | token == "and" | token == "or"){
        return true;
    }
    else{
        return false;
    }
}

/**
 * Generate sql from tag query
 */
std::string Db::GenSql(std::string tag_query) {

    std::vector<std::string> tokens = this->Tokenize(tag_query);

    if(tokens.size() == 0){
        return "select path from file_tags group by path;";
    }

    int idx = 0;
    int idx_max = tokens.size()-1;

    std::string result = "";

    while(idx <= idx_max){

        if(tokens[idx] == "not"){
            if(idx + 1 > idx_max){
                throw std::invalid_argument("Error: invalid tag query, not cannot be last token");
            }
            else{
                idx++;
                result += " select path from file_tags left join ( select path from file_tags where tag == \"" + tokens[idx] + "\" ) as t using(path) where t.path is null";
                idx++;
            }
        }
        else if(tokens[idx] == "and"){
            if(idx == 0){
                throw std::invalid_argument("Error: invalid tag query, and cannot be first token");
            }
            else{
                result += " intersect ";
                idx++;
            }
        }
        else if(tokens[idx] == "or"){
            if(idx == 0){
                throw std::invalid_argument("Error: invalid tag query, or cannot be first token");
            }
            else{
                result += " union ";
                idx++;
            }
        }
        else{
            result += " select path from file_tags where tag == \"" + tokens[idx] + "\" ";
            idx++;
        }
    }

    result += " group by path;";

    return result;
}
