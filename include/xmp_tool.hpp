#ifndef XMP_TOOLS_H_
#define XMP_TOOLS_H_

#include <vector>
#include <set>

/**
 * Get tags from a file
 */
std::vector<std::string> GetTagsFromFile(std::string file_path);

/**
 * Get tags from a file
 */
void ReadXmpFromFile(std::string file_path);

/**
 * Add tags to file
 */
bool AddTagsToFile(std::string file_path, std::set<std::string>& tags);

/**
 * Add tags to files
 */
void AddTagsToFiles(std::vector<std::string>& paths, std::set<std::string>& tags);

/**
 * Remove tags from file
 */
bool RemoveTagsFromFile(std::string file_path, std::set<std::string>& tags);

/**
 * Remove tags from files
 */
void RemoveTagsFromFiles(std::vector<std::string>& paths, std::set<std::string>& tags);

/**
 * Remove all tags from file
 */
bool RemoveAllTagsFromFile(std::string file_path);

/**
 * Remove all tags from files
 */
void RemoveAllTagsFromFiles(std::vector<std::string>& paths);

/**
 * Remove duplicate tags from file
 */
bool RemoveDuplicateTagsFromFile(std::string file_path);

/**
 * Remove duplicate tags from files
 */
void RemoveDuplicateTagsFromFiles(std::vector<std::string>& paths);

/**
 * Get all file paths within a directory
 */
std::vector<std::string> GetFilePaths(std::string dir_path, bool recurse);

/**
 * Get and store tags for all paths in a vector into a database
 */
void GetAndStoreTags(std::vector<std::string>& paths, std::string db_path);

/**
 * Get all paths that have a tag 
 */
void PrintPathsForTagQuery(std::string db_path, std::string tag);

#endif // XMP_TOOLS_H_
