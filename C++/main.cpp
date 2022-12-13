#include"./function/classNode.hpp"

#include <iostream>
#include <fstream>

#include <cstring>
#include <vector>
#include <queue>
#include <bitset>

using error_code_t = int;
using queueNodePointer = std::priority_queue<Node::pointer , std::vector<Node::pointer>, LowestPriority>;


namespace ns_file
{
    enum eFileError : unsigned char
    {
        ERR_UNKNOWN = 1
        ,NO = 0
        ,ERR_OPEN = 2
        ,ERR_CLOSE = 3
        ,FILE_CLEAR = 255
    };
};

std::ifstream::pos_type getSizeFile(const std::string& _file);
error_code_t getFrequency(std::vector<int>& _frequency, std::string _pathFile);

void filingQueueFrequency(queueNodePointer& _queue, std::vector<int> _frequency);
void buildTree(queueNodePointer& _queue);
void makeCode(Node::pointer& _node, std::string _str, std::vector<std::string>& _codes);
std::string getMessageCode(const std::string& _pathFile, const std::vector<std::string>& _codes);
error_code_t writeFile(const std::string& _pathFile,std::vector<int>& _frequency,const queueNodePointer& _quenue,const std::string& _messange);

error_code_t readFile(const std::string& _pathFile,std::vector<int>& _frequency,std::string& _messange);
void makeChar(const Node::pointer& _root, std::string& _messange, std::string& _text);
error_code_t writeDecodeFile(std::string _pathFile, std::string _text);

int main()
{
    using namespace std;

    string pathFile = "./text/input.txt";

    /*
    Открытие файла для заполнения частот
    */

    vector<int> frequency(255, 0); 
    
    cout << "READING FILE" << endl;
    getFrequency(frequency, pathFile);
    cout << endl;
    cout << "COMPRES" << endl;
    cout << "..." << endl;
    
    /*
    Создаем очередь приоритетов
    */

    queueNodePointer queue;
    filingQueueFrequency(queue, frequency);

    //Создаем дерево

    buildTree(queue);
    
    //Создаем коды
    
    vector<string> codes(255, "");
    Node::pointer root = queue.top();
    makeCode(root, "", codes);

    //Кодируем файл

    string messageCode = getMessageCode(pathFile, codes);

    //Запись результата в файл
    cout << "..." << endl;

    writeFile(pathFile, frequency, queue, messageCode);   

    /*
    Обратное кодирование
    */
    cout << "..." << endl;

    //Частоты и код
    vector<int> frequency2(255, 0);
    string messageCode2 = getMessageCode(pathFile, codes);

    readFile(pathFile + ".hff.code", frequency2, messageCode2);

    //Дерево
    queueNodePointer queue2;
    filingQueueFrequency(queue2, frequency2);
    buildTree(queue2);

    Node::pointer root2 = queue2.top();
    string text = "";
    
    //идем и преобразовываем обратно
    makeChar(root2, messageCode2, text);
    cout << "..." << endl;
    writeDecodeFile(pathFile, text);

    cout << "COMPLETE" << endl;
    cout << endl;
    
    return 0;
};

std::ifstream::pos_type getSizeFile(const std::string& _file)
{
    std::ifstream file(_file, std::ifstream::ate);
    return file.tellg();
};

error_code_t getFrequency(std::vector<int>& _frequency, std::string _pathFile)
{
    int fileSize = getSizeFile(_pathFile);
    if (fileSize < 0)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]:" << strerror(errno) << std::endl;
        return ns_file::FILE_CLEAR;
    };

    std::ifstream file(_pathFile, std::ifstream::binary);
    if(!file)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return ns_file::ERR_OPEN;
    };

    int i = 0;
    while(true)
    {        
        char ch;
        file.read(&ch, 1);

        //Баг с последним символом
        if(file.eof())
        {
            break;
        };

        _frequency[static_cast<unsigned char>(ch)]++;

        int var = (i+ fileSize * 100.0) / fileSize;
        std::cout << "\r" << var << "%" << std::flush;
        ++i;
    };

    return 0;
};

void makeCode(Node::pointer& _node, std::string _str, std::vector<std::string>& _codes)
{

    if(_node->left != nullptr)
    {
        makeCode(_node->left, _str + '0', _codes);
    };

    if(_node->right != nullptr)
    {
        makeCode(_node->right, _str + '1', _codes);
    };

    if(_node->right == nullptr && _node->left == nullptr)
    {
        _codes[_node->getSymbol()] = _str;
        _node->setCode(_str);
    };
};

void filingQueueFrequency(queueNodePointer& _queue, std::vector<int> _frequency)
{
    for(int i = 0; i < _frequency.size(); i++)
    {
        if(_frequency[i])
        {   
            Node::pointer node = std::make_shared<Node>(i, _frequency[i]);
            _queue.push(node);
        };
    };
};

void buildTree(queueNodePointer& _queue)
{
    while (_queue.size() > 1)
    {
        Node::pointer x = _queue.top();
        _queue.pop();

        Node::pointer y = _queue.top();
        _queue.pop();

        std::string name = x->getName() + y->getName();
        Node::pointer z = std::make_shared<Node>(name, x->getFrequency() + y->getFrequency());
        z->left = x;
        z->right = y;

        x->parent = z;
        y->parent = z;

        _queue.push(z);
    };
};

std::string getMessageCode(const std::string& _pathFile, const std::vector<std::string>& _codes)
{
    std::string msg {""};

    std::ifstream file(_pathFile, std::ifstream::binary);
    if(!file)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return msg;
    };

    while(true)
    {        
        char ch;
        file.read(&ch, 1);

        //Баг с последним символом
        if(file.eof())
        {
            break;
        };

        msg += _codes[static_cast<ubyte>(ch)];
    };

    file.close();
    if(file)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return msg;
    };

    return msg;
};  

error_code_t writeFile(
    const std::string& _pathFile
    ,std::vector<int>& _frequency
    ,const queueNodePointer& _quenue
    ,const std::string& _messange)
{   
    std::string pathResultFile = _pathFile + ".hff.code";

    std::ofstream resultFile(pathResultFile, std::ofstream::binary);
    if(!resultFile)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return ns_file::ERR_OPEN ;
    };

    //Кол-во элементов
    ubyte count = _quenue.top()->getName().length();
    resultFile.write(reinterpret_cast<char*>(&count), sizeof(count));

    for(ubyte index = 0; index < _frequency.size(); index++)
    {
        int value = _frequency[index];
        if(value != 0)
        {
            resultFile.write(reinterpret_cast<char*>(&index), sizeof(index));
            resultFile.write(reinterpret_cast<char*>(&value), sizeof(value));
        };
    };

    int byteCount = _messange.size() / __CHAR_BIT__;
    int moduls = _messange.size() % __CHAR_BIT__;

    resultFile.write(reinterpret_cast<char*>(&byteCount), byteCount);
    resultFile.write(reinterpret_cast<char*>(&moduls), moduls);

    int i = 0;
    for(; i < byteCount; i++)
    {
        std::bitset<__CHAR_BIT__> b(_messange.substr(i * __CHAR_BIT__, __CHAR_BIT__));
        ubyte value = static_cast<ubyte>(b.to_ulong());
        resultFile.write(reinterpret_cast<char*>(&value), sizeof(value));
    };

    if(moduls > 0)
    {
        std::bitset<__CHAR_BIT__> b(_messange.substr(i * __CHAR_BIT__, __CHAR_BIT__));
        ubyte value = static_cast<ubyte>(b.to_ulong());
        resultFile.write(reinterpret_cast<char*>(&value), sizeof(value));
    };

    return 0;
};

error_code_t readFile(
    const std::string& _pathFile
    ,std::vector<int>& _frequency
    ,std::string& _messange
    )
{
    std::ifstream file(_pathFile, std::ifstream::binary);
    if(!file)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return ns_file::ERR_OPEN ;
    };

    ubyte count = 0;
    file.read(reinterpret_cast<char*>(&count), sizeof(count));

    int i = 0;
    while (i < count)
    {
        ubyte ch;
        file.read(reinterpret_cast<char*>(&ch), sizeof(ch));

        int f = 0;
        file.read(reinterpret_cast<char*>(&f), sizeof(f));

        _frequency[ch] = f;
        ++i;
    };

    int byteCount = 0;
    int moduls =0;

    file.read(reinterpret_cast<char*>(&byteCount), byteCount);
    file.read(reinterpret_cast<char*>(&moduls), moduls);

    i = 0;
    for(; i < byteCount; i++)
    {
        char byte;
        file.read(reinterpret_cast<char*>(&byte), sizeof(byte));

        std::bitset<__CHAR_BIT__> b(byte);

        _messange += b.to_string();
    };

    if(moduls > 0)
    {
        char byte;
        file.read(reinterpret_cast<char*>(&byte), sizeof(byte));

        std::bitset<__CHAR_BIT__> b(byte);

        _messange += b.to_string().substr(__CHAR_BIT__ - moduls, __CHAR_BIT__);
    };
    
    return 0;
};

void makeChar(
    const Node::pointer& _root, 
    std::string& _messange, 
    std::string& _text
    )
{
    Node::pointer node = _root;

    for(int i = 0; i < _messange.size(); i++)
    {
        char ch = _messange[i];

        if(ch == '0')
        {
            if(node->left != nullptr)
            {
                node = node->left;

                if(node->left == nullptr && node->right == nullptr)
                {
                    _text += node->getSymbol();
                    node = _root;
                };
            };
            continue;
        };

        if(ch == '1')
        {
            if(node->right != nullptr)
            {
                node = node->right;
                
                if(node->left == nullptr && node->right == nullptr)
                {
                    _text += node->getSymbol();
                    node = _root;
                };
            };
            continue;
        };
    };
    
};

error_code_t writeDecodeFile(std::string _pathFile, std::string _text)
{
    std::string pathResultFile = _pathFile + ".hff.decode";

    std::ofstream resultFile(pathResultFile, std::ofstream::binary);
    if(!resultFile)
    {
        std::cerr << "Error in [" << __PRETTY_FUNCTION__ << "]: " << strerror(errno) << std::endl;
        return ns_file::ERR_OPEN ;
    };

    resultFile << _text;

    return 0;
};








