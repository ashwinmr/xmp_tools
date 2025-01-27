#include "xmp_file.hpp"
#include "db_tools.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <set>

// Must be defined to instantiate template classes
#define TXMP_STRING_TYPE std::string

// Must be defined to give access to XMPFiles
#define XMP_INCLUDE_XMPFILES 1

// Ensure XMP templates are instantiated
#include "XMP.incl_cpp"

// Provide access to the API
#include "XMP.hpp"

// Forward declarations
XMP_Status DumpXmpToConsole(void * not_used, XMP_StringPtr buffer, XMP_StringLen bufferSize);

namespace fs = boost::filesystem;

/**
 * Get tags from a file
 */
std::vector<std::string> GetTagsFromFile(std::string file_path) {
    std::vector<std::string> tags;

    XmpFile xmp_file(file_path, true, false);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();

        // Get the the entire dc:subject array
        int n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
        for (int i = 1; i <= n_tags; i++) {
            std::string tag;
            meta.GetArrayItem(kXMP_NS_DC, "subject", i, &tag, 0);
            tags.push_back(tag);
        }
    }

    return tags;
}

/**
 * Dump xmp to console callback
 */
XMP_Status DumpXmpToConsole(void * not_used, XMP_StringPtr buffer, XMP_StringLen bufferSize)
{
	XMP_Status status = 0;
	
	try
	{
        std::cout << buffer;
	}
	catch(XMP_Error & e)
	{
        std::cout << "Error reading xmp data:\n\t" << e.GetErrMsg() << std::endl;
		return -1;  // Return a bad status
	}

	return status;
}

/**
 * Read xmp data from file
 */
void ReadXmpFromFile(std::string file_path) {

    XmpFile xmp_file(file_path, true, false);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();

        meta.DumpObject(DumpXmpToConsole, NULL);
    }

    return;
}

/**
 * Add tags to file
 */
bool AddTagsToFile(std::string file_path, std::set<std::string> &tags) {
    bool success = false;

    // If not tags provided return success
    if (tags.empty()) {
        success = true;
        return success;
    }

    XmpFile xmp_file(file_path, false, true);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();
        
        // Determine existing tags
        int n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
        std::set<std::string> tags_existing;
        for (int i = 1; i <= n_tags; i++) {
            std::string tag;
            meta.GetArrayItem(kXMP_NS_DC, "subject", i, &tag, 0);
            tags_existing.insert(tag);
        }

        // Add tags that don't exist
        for (auto &tag : tags) {
            if(tags_existing.find(tag) == tags_existing.end()){
                // Note the options used, kXMP_PropArrayIsOrdered, if the array does not exist it will be created
                meta.AppendArrayItem(kXMP_NS_DC, "subject", kXMP_PropArrayIsOrdered, tag, 0);
            }
        }

        if(xmp_file.PutMeta(meta)){
            success = true;
        }
    }

    return success;
}

/**
 * Add tags to files
 */
void AddTagsToFiles(std::vector<std::string>& paths, std::set<std::string>& tags){

    for(auto& path: paths){
        if(!AddTagsToFile(path,tags)){
            std::cout << "Warning: Failed to add tags to " << path << std::endl;
        } 
    }
}

/**
 * Remove tags from file
 */
bool RemoveTagsFromFile(std::string file_path, std::set<std::string> &tags) {
    bool success = false;

    XmpFile xmp_file(file_path, false, true);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();

        // Now modify the XMP

        int n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
        int i = 1;

        // Deleting a tag, changes the array size
        while(i <= n_tags){
            std::string tag;
            meta.GetArrayItem(kXMP_NS_DC, "subject", i, &tag, 0);
            
            if(tags.find(tag) != tags.end()){
                meta.DeleteArrayItem(kXMP_NS_DC, "subject", i);
                n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
            }
            else{
                i++;
            }
        }

        if(xmp_file.PutMeta(meta)){
            success = true;
        }
    }
    return success;
}

/**
 * Remove all tags from file
 */
bool RemoveAllTagsFromFile(std::string file_path) {
    bool success = false;

    XmpFile xmp_file(file_path, false, true);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();

        // Now modify the XMP

        int n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");

        // Deleting a tag, changes the array size
        while(n_tags > 0){

            meta.DeleteArrayItem(kXMP_NS_DC, "subject", 1);
            n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
        }

        if(xmp_file.PutMeta(meta)){
            success = true;
        }
    }
    return success;
}

/**
 * Remove duplicate tags from file
 */
bool RemoveDuplicateTagsFromFile(std::string file_path) {
    bool success = false;

    XmpFile xmp_file(file_path, false, true);

    if(xmp_file.valid){
        // Create the xmp object and get the xmp data
        SXMPMeta meta = xmp_file.GetMeta();

        // Now modify the XMP

        int n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
        int i = 1;
        std::set<std::string> tags;

        // Deleting a tag, changes the array size
        while(i <= n_tags){
            std::string tag;
            meta.GetArrayItem(kXMP_NS_DC, "subject", i, &tag, 0);
            
            if(tags.find(tag) != tags.end()){
                meta.DeleteArrayItem(kXMP_NS_DC, "subject", i);
                n_tags = meta.CountArrayItems(kXMP_NS_DC, "subject");
            }
            else{
                tags.insert(tag);
                i++;
            }
        }

        if(xmp_file.PutMeta(meta)){
            success = true;
        }
    }
    return success;
}

/**
 * Remove tags from files
 */
void RemoveTagsFromFiles(std::vector<std::string>& paths, std::set<std::string>& tags){

    for(auto& path: paths){
        if(!RemoveTagsFromFile(path,tags)){
            std::cout << "Warning: Failed to remove tags from " << path << std::endl;
        } 
    }
}

/**
 * Remove all tags from files
 */
void RemoveAllTagsFromFiles(std::vector<std::string>& paths){

    for(auto& path: paths){
        if(!RemoveAllTagsFromFile(path)){
            std::cout << "Warning: Failed to remove all tags from " << path << std::endl;
        } 
    }
}

/**
 * Remove duplicate tags from files
 */
void RemoveDuplicateTagsFromFiles(std::vector<std::string>& paths){

    for(auto& path: paths){
        if(!RemoveDuplicateTagsFromFile(path)){
            std::cout << "Warning: Failed to remove duplicate tags from " << path << std::endl;
        } 
    }
}

/**
 * Get all file paths within a directory
 */
std::vector<std::string> GetFilePaths(std::string dir_path, bool recurse) {
    std::vector<std::string> paths;

    // Check if path is a directory
    if (!fs::is_directory(dir_path)) {
        std::cout << "Input path is not directory" << std::endl;
        return paths;
    }

    if (recurse) {
        for (fs::directory_entry &de : fs::recursive_directory_iterator(dir_path)) {
            if (fs::is_regular_file(de.path())) {
                paths.push_back(de.path().string());
            }
        }
    } else {
        for (fs::directory_entry &de : fs::directory_iterator(dir_path)) {
            if (fs::is_regular_file(de.path())) {
                paths.push_back(de.path().string());
            }
        }
    }

    return paths;
}

/**
 * Get and store tags for all paths in a vector into a database
 */
void GetAndStoreTags(std::vector<std::string>& paths, std::string db_path) {

    // Create and open database
    Db db(db_path);
    if (!db.IsOpen()) {
        return;
    }

    // Insert into database
    for (auto &path : paths) {
        std::vector<std::string> tags = GetTagsFromFile(path);
        if (!tags.empty()) {
            for (auto &tag : tags) {
                db.InsertRow(path, tag);
            }
        }
    }
}

/**
 * Select rows
 */
void PrintPathsForTagQuery(std::string db_path, std::string tag_query) {
    // Check if database doesnt exist
    if (!fs::is_regular_file(db_path)) {
        std::cout << "Database doesn't exist" << std::endl;
        return;
    }

    // Open database
    Db db(db_path);
    if (!db.IsOpen()) {
        return;
    }

    // Select from database
    std::vector<std::string> paths = db.SelectTagQuery(tag_query);

    // Print paths
    for (auto &path : paths) {

        // Escape spaces before printing
        std::string path_esc = "";
        for(auto &ch: path){
            if(ch != ' '){
                path_esc += ch;
            }
            else{
                path_esc += "\\ ";
            }
        }
        std::cout << path_esc << std::endl;
    }
}
