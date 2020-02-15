#include <iostream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include "gitint.h"

using namespace std;

/*********************** Messages to use for errors ***************************/
const std::string INVALID_COMMAND = "Invalid command";
const std::string INVALID_OPTION = "Invalid option";
const std::string INVALID_COMMIT_NUMBER = "Invalid commit number";
const std::string LOG_COMMIT_STARTER = "Commit: ";



// Class implementation

void GitInt::print_menu() const
{
    cout << "Menu:                          " << endl;
    cout << "===============================" << endl;
    cout << "create   filename int-value    " << endl;
    cout << "edit     filename int-value    " << endl;
    cout << "display  (filename)            " << endl;
    cout << "display  commit-num            " << endl;
    cout << "add      file1 (file2 ...)     " << endl;
    cout << "commit   \"log-message\"       " << endl;
    cout << "tag      (-a tag-name)         " << endl;
    cout << "log                            " << endl;
    cout << "checkout commit-num/tag-name   " << endl;
    cout << "diff                           " << endl;
    cout << "diff     commit                " << endl;
    cout << "diff     commit-n commit-m     " << endl;
}


bool GitInt::process_command(std::string cmd_line)
{
    bool quit = false;
    std::stringstream ss(cmd_line);
    std::string cmd;
    ss >> cmd;
    if (ss.fail()) throw std::runtime_error(INVALID_COMMAND);

    if (cmd == "quit") {
        quit = true;
    } else if(cmd == "create") {
        std::string filename;
        int value;
        if(ss >> filename && ss >> value) {
            create(filename, value);
        } else {
            throw std::invalid_argument(INVALID_COMMAND);
        }
    } else if(cmd == "edit") {
        std::string filename;
        int value;
        if(ss >> filename && ss >> value) {
            edit(filename, value);
        } else {
            throw std::invalid_argument(INVALID_COMMAND);
        }
    } else if(cmd == "display") {
        string filename;
        int index = 0, flag = 0;
        if(ss >> filename) {
            try {
                index = stoi(filename);
                flag = 1;
            } catch (std::invalid_argument& e) {
                display(filename);
            }
            if(flag) display_commit(index);
        } else {
            display_all();
        }
    } else if(cmd == "add") {
        string filename;
        while(ss >> filename) {
            add(filename);
        }
    } else if(cmd == "commit") {
        string message;
        getline(ss, message);
        size_t start = message.find_first_of('\"');
        if(start == string::npos) {
            throw runtime_error(INVALID_COMMAND);
        }
        message = message.substr(start+1);
        size_t end = message.find_first_of('\"');
        if(end == string::npos) {
            throw runtime_error(INVALID_COMMAND);
        }
        message = message.substr(0,end);
        commit(message);
    } else if(cmd == "tag") {
        string option, tagname;
        if(ss >> option) {
            if(option == "-a") {
                if(ss >> tagname)  create_tag(tagname,current);
                else throw runtime_error(INVALID_COMMAND);
            } else throw runtime_error(INVALID_COMMAND);

        } else {
            tags();
        }
    } else if(cmd == "log") {
        log();
    } else if(cmd == "checkout") {
        string tagname;
        int flag = 0, tag = 0;
        if(ss >> tagname) {
            try {
                tag = stoi(tagname);
                flag = 1;
            } catch (std::invalid_argument &e) {
                checkout(tagname);
            }
            if(flag) checkout(tag);
        } else {
            throw runtime_error(INVALID_COMMAND);
        }
    } else if(cmd == "diff") {
        int commit1, commit2;
        if(ss >> commit1) {
            if(ss >> commit2) {
                if(commit1 < commit2) throw invalid_argument(INVALID_COMMAND);
                diff(commit1, commit2);
            } else {
                diff(commit1);
            }
        } else {
            display_helper(diff_helper(currentFiles,untracked));
        }
    } else {
        throw runtime_error(INVALID_COMMAND);
    }


    return quit;
}

void GitInt::display_commit(CommitIdx commit) const
{
    if ( false == valid_commit(commit) ) {
        throw std::invalid_argument(INVALID_COMMIT_NUMBER);
    }
    display_helper(commits_[commit].diffs_);
}


void GitInt::display_helper(const std::map<std::string, int>& dat) const
{
    for (std::map<std::string, int>::const_iterator cit = dat.begin();
            cit != dat.end();
            ++cit) {
        std::cout << cit->first << " : " << cit->second << std::endl;
    }
}


void GitInt::log_helper(CommitIdx commit_num, const std::string& log_message) const
{
    std::cout << LOG_COMMIT_STARTER << commit_num << std::endl;
    std::cout << log_message << std::endl << std::endl;

}

void GitInt::create(const std::string &filename, int value) {
    if(untracked.find(filename) != untracked.end()) {
        throw std::invalid_argument(INVALID_COMMAND);
    }
    untracked[filename] = value;
}

void GitInt::edit(const std::string &filename, int value) {
    if(untracked.find(filename) == untracked.end()) {
        throw std::invalid_argument(INVALID_COMMAND);
    }
    untracked[filename] = value;
}

void GitInt::display(const std::string &filename) const {
    if(untracked.find(filename) == untracked.end()) {
        throw std::invalid_argument(INVALID_COMMAND);
    }
    cout << untracked.at(filename) << endl;
}

void GitInt::display_all() const {
    display_helper(untracked);
}

void GitInt::add(std::string filename) {
    if(untracked.find(filename) == untracked.end()) {
        throw std::invalid_argument(INVALID_COMMAND);
    }
    stages.insert(filename);
}

void GitInt::commit(std::string message) {
    if(stages.empty()) throw runtime_error("");

    std::map<std::string, int> temp;
    std::map<std::string, int> newFiles = currentFiles;
    for(const string& file : stages) {
        if(untracked.find(file) != untracked.end()) {
            temp[file] = untracked[file];
            newFiles[file] = untracked[file];
        }
    }
    stages.clear();
    const std::map<std::string, int>& diffs = diff_helper(currentFiles, temp);
    commits_.emplace_back(message, diffs, current);
    current = commits_.size()-1;
    currentFiles = newFiles;
}

void GitInt::create_tag(const std::string &tagname, CommitIdx commit) {
    if(tags_map.find(tagname) != tags_map.end()) {
        throw invalid_argument(INVALID_COMMAND);
    }
    tags_map[tagname] = commit;
    tags_.emplace_back(tagname);
}

void GitInt::tags() const {
    vector<string>::const_iterator it = tags_.begin();
    while(it < tags_.end()) {
        cout << (*it) << endl;
        it++;
    }
}

bool GitInt::checkout(CommitIdx commitIndex) {
    if(valid_commit(commitIndex)) {
        currentFiles = checkout_helper(commitIndex);
        untracked = currentFiles;
        current = commitIndex;
        return true;
    }
    throw invalid_argument(INVALID_COMMAND);
}

bool GitInt::checkout(std::string tag) {
    if(tags_map.find(tag) != tags_map.end())
        return checkout(tags_map[tag]);
    throw invalid_argument(INVALID_COMMAND);
}

void GitInt::log() const {
    for(int i = current; i > 0; i = commits_[i].parent_) {
        log_helper(i,commits_[i].msg_);
    }
}

void GitInt::diff(CommitIdx to) const {
    const map<string,int>& temp = checkout_helper(to);
    const map<string,int>& diff = diff_helper(temp,untracked);
    display_helper(diff);
}

void GitInt::diff(CommitIdx from, CommitIdx to) const {
    const map<string,int>& from1 = checkout_helper(from);
    const map<string,int>& to1 = checkout_helper(to);
    const map<string,int>& diff = diff_helper(to1, from1);
    display_helper(diff);
}

bool GitInt::valid_commit(CommitIdx commit) const {
    return commit > 0 && commit <= (int)commits_.size();
}

std::map<std::string, int> GitInt::buildState(CommitIdx from, CommitIdx to) const {
    return std::map<std::string, int>();
}

GitInt::GitInt() {
    commits_.emplace_back("init", currentFiles, -1);
    current = 0;
}

std::map<std::string, int> GitInt::checkout_helper(CommitIdx commitIdx) const {
    vector<map<string,int> > parents;
    while(commitIdx > 0) {
        parents.emplace_back(commits_[commitIdx].diffs_);
        commitIdx = commits_[commitIdx].parent_;
    }
    vector<map<string,int> >::iterator it = parents.end()-1;
    map<string,int> files = *it;
    it--;
    for(;it >= parents.begin(); it--) {
        for(pair<string,int> pair: *it) {
            if(files.find(pair.first) == files.end()) {
                files[pair.first] = pair.second;
            } else {
                files[pair.first] += pair.second;
            }
        }
    }
    return files;
}

std::map<std::string, int> GitInt::diff_helper(std::map<std::string, int> files1, std::map<std::string, int> files2) const {
    std::map<string,int> diffs;
    for(pair<string,int> pair : files2) {
        if(files1.find(pair.first) != files1.end()) {
            if(files2[pair.first] - files1[pair.first]) {
                diffs[pair.first] = files2[pair.first] - files1[pair.first];
            }
        } else {
            diffs[pair.first] = files2[pair.first];
        }
    }
    for(pair<string,int> pair : files1) {
        if(files2.find(pair.first) == files2.end()) {
            diffs[pair.first] = -files1[pair.first];
        }
    }
    return diffs;
}

