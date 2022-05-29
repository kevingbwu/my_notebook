#include <bits/stdc++.h>

using namespace std;

class Message;

class Folder {
    friend void swap(Folder &lhs, Folder &rhs);
public:
    Folder() = default;
    Folder(const Folder &f);
    Folder& operator=(const Folder &);
    void addMsg(Message *msg) {
        messages.insert(msg);
    }
    void remMsg(Message *msg) {
        messages.erase(msg);
    }
    ~Folder();
private:
    void add_to_Message(const Folder &f);
    void remove_from_Message();
    set<Message *> messages;
};

class Message {
    friend void swap(Message &lhs, Message &rhs);
public:
    // folders is implicitly initialized to the empty set
    explicit Message(const std::string &str = ""):
            contents(str) { }
    // copy control to manage pointers to this Message
    Message(const Message&); // copy constructor
    Message& operator=(const Message&); // copy assignment
    ~Message(); // destructor
    // add/remove this Message from the specified Folder's set of messages
    void save(Folder&);
    void remove(Folder&);
    void addFolder(Folder *folder) {
        folders.insert(folder);
    }
    void remFolder(Folder *folder) {
        folders.erase(folder);
    }
private:
    std::string contents; // actual message text
    std::set<Folder*> folders; // Folders that have this Message
    // utility functions used by copy constructor, assignment, and destructor
    // add this Message to the Folders that point to the parameter
    void add_to_Folders(const Message&);
    // remove this Message from every Folder in folders
    void remove_from_Folders();
};

void Folder::add_to_Message(const Folder &f)
{
    for (auto m : f.messages)
        m->addFolder(this);
}

Folder::Folder(const Folder &f)
        : messages(f.messages)
{
    add_to_Message(f);
}

void Folder::remove_from_Message()
{
    for (auto m : messages)
        m->remFolder(this);
}

Folder::~Folder() {
    remove_from_Message();
}

Folder& Folder::operator=(const Folder &rhs) {
    remove_from_Message();
    messages = rhs.messages;
    add_to_Message(*this);
    return *this;
}

void swap(Folder &lhs, Folder &rhs)
{
    using std::swap;
    lhs.remove_from_Message();
    rhs.remove_from_Message();

    swap(lhs.messages, rhs.messages);

    lhs.add_to_Message(lhs);
    rhs.add_to_Message(rhs);
}

void Message::save(Folder &f)
{
    folders.insert(&f); // add the given Folder to our list of Folders
    f.addMsg(this); // add this Message to f's set of Messages
}

void Message::remove(Folder &f)
{
    folders.erase(&f); // take the given Folder out of our list of Folders
    f.remMsg(this); // remove this Message to f's set of Messages
}

// add this Message to Folders that point to m
void Message::add_to_Folders(const Message &m)
{
    for (auto f : m.folders) // for each Folder that holds m
        f->addMsg(this); // add a pointer to this Message to that Folder
}

Message::Message(const Message &m):
        contents(m.contents), folders(m.folders)
{
    add_to_Folders(m); // add this Message to the Folders that point to m
}

// remove this Message from the corresponding Folders
void Message::remove_from_Folders()
{
    for (auto f : folders) // for each pointer in folders
        f->remMsg(this); // remove this Message from that Folder
}

Message::~Message()
{
    remove_from_Folders();
}

Message& Message::operator=(const Message &rhs)
{
    // handle self-assignment by removing pointers before inserting them
    remove_from_Folders(); // update existing Folders
    contents = rhs.contents; // copy message contents from rhs
    folders = rhs.folders; // copy Folder pointers from rhs
    add_to_Folders(rhs); // add this Message to those Folders
    return *this;
}

void swap(Message &lhs, Message &rhs)
{
    using std::swap; // not strictly needed in this case, but good habit
    // remove pointers to each Message from their (original) respective Folders
    for (auto f: lhs.folders)
        f->remMsg(&lhs);
    for (auto f: rhs.folders)
        f->remMsg(&rhs);
    // swap the contents and Folder pointer sets
    swap(lhs.folders, rhs.folders); // uses swap(set&, set&)
    swap(lhs.contents, rhs.contents); // swap(string&, string&)
    // add pointers to each Message to their (new) respective Folders
    for (auto f: lhs.folders)
        f->addMsg(&lhs);
    for (auto f: rhs.folders)
        f->addMsg(&rhs);
}

int main() {
    Message msg("abc");
    Folder fld;
    msg.save(fld);
    return 0;
}
