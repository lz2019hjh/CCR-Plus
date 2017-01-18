#ifndef PROBLEM_H
#define PROBLEM_H

#include <QMap>

#include "common/const.h"

class Compiler
{
public:
    explicit Compiler(const QString& cmd, const QString& file, int time = 10) :
        cmd(cmd), file(file), time_lim(time) {}

    explicit Compiler(const Compiler& compiler) :
        cmd(compiler.cmd), file(compiler.file), time_lim(compiler.time_lim) {}

    // Getter member functions
    QString Cmd() const { return cmd; }
    QString SourceFile() const { return file; }
    int TimeLimit() const { return time_lim; }

    static const int BUILTIN_COMPILER_COUNT = 3;
    static const QStringList BUILTIN_COMPILER_CMD;
    static const QStringList BUILTIN_COMPILER_FILE;

private:
    QString cmd, file;
    int time_lim;
};



class TestCase
{
public:
    explicit TestCase(double t, double m, const QString& in, const QString& out, const QString& sub = "") :
        time_lim(t), mem_lim(m), in(in), out(out), sub(sub) {}

    explicit TestCase(const TestCase& point) :
        time_lim(point.time_lim), mem_lim(point.mem_lim), in(point.in), out(point.out), sub(point.sub) {}

    // Getter member functions
    double TimeLimit() const { return time_lim; }
    double MemoryLimit() const { return mem_lim; }
    QString InFile() const { return in; }
    QString OutFile() const { return out; }
    QString SubmitFile() const { return sub; }

    // Setter member functions
    void SetTimeLimit(double lim) { time_lim = lim; }
    void SetMemoryLimit(double lim) { mem_lim = lim; }
    void SetInFile(const QString& _in) { in = _in; }
    void SetOutFile(const QString& _out) { out = _out; }
    void SetSubmitFile(const QString& _sub) { sub = _sub; }

private:
    double time_lim, mem_lim;
    QString in, out, sub;
};



struct Subtask
{
public:
    explicit Subtask(int s) : score(s) {}

    // Getter member functions
    int Score() const { return score; }
    int Size() const { return cases.size(); }

    /// 添加测试点
    void Append(TestCase* point) { cases.append(point); }

    // Iterators
    QList<TestCase*>::const_iterator begin() const { return cases.begin(); }
    QList<TestCase*>::const_iterator end() const { return cases.end(); }

private:
    const int score;
    QList<TestCase*> cases;
};



class Problem
{
public:
    explicit Problem(const QString& name = "");
    explicit Problem(Problem* problem);
    ~Problem() { Clear(); }

    /// 内置校验器列表，格式： <file, <name, toolTip>>
    static const QMap<QString, QPair<QString, QString>> BUILTIN_CHECKER_MAP;

    /// 增加扩展名
    static QString AddFileExtension(QString file)
    {
#ifdef Q_OS_WIN
        if (!file.endsWith(".exe")) file += ".exe";
#endif
        return file;
    }

    /// 去除扩展名
    static QString RemoveFileExtension(QString file)
    {
#ifdef Q_OS_WIN
        if (file.endsWith(".exe")) file.remove(file.length() - 4, 4);
#endif
        return file;
    }

    /// 尝试从内置校验器名称获取校验器文件名，不是内置校验器返回原始文件名
    static QString FromBuiltinCheckerName(const QString& name);

    /// 是否是内置校验器
    static bool IsBuiltinChecker(const QString& checker)
    {
        return BUILTIN_CHECKER_MAP.find(RemoveFileExtension(checker)) != BUILTIN_CHECKER_MAP.end();
    }

    // Getter member functions
    QString Name() const { return name; }
    QString Directory() const { return dir; }
    QString ExecutableFile() const { return exe; }
    QString Checker() const { return checker; }
    QString InFile() const { return in_file; }
    QString OutFile() const { return out_file; }
    int Score() const { return score; }
    int CheckerTimeLimit() const { return checker_time_lim; }
    double CodeLengthLimit() const { return code_len_lim; }
    Global::ProblemType Type() const { return type; }

    // Vector traversal interface
    int TestCaseCount() const { return cases.size(); }
    int SubtaskCount() const { return subtasks.size(); }
    int CompilerCount() const { return compilers.size(); }
    TestCase* TestCaseAt(int t) const { return cases[t]; }
    Subtask* SubtaskAt(int t) const { return subtasks[t]; }
    Compiler* CompilerAt(int t) const { return compilers[t]; }

    // Compiler operations
    void InsertCompiler(int p, Compiler* compiler) { compilers.insert(p, compiler); }
    void RemoveCompiler(int p)
    {
        delete compilers[p];
        compilers.removeAt(p);
    }
    void MoveCompiler(int from, int to) { compilers.move(from, to); }

    /// 清空
    void ClearCompilers();
    void ClearTestCases();
    void Clear();

    /// 读入配置文件
    void ReadConfiguration();

    /// 保存配置文件
    bool SaveConfiguration();

    /// 重置编译器
    void ResetCompilers();

    /// 重置测试点
    void ResetTestCases(double timeLim, double memLim);

    /// 配置试题，只修改时间限制 、内存限制、校验器等
    void Configure(const QString& typ, double timeLim, double memLim, const QString& check);

    /// 清空原来的配置，重新配置试题
    void ConfigureNew(const QString& typ, double timeLim, double memLim, const QString& check);

    /// 根据选手目录下的源文件自动选择编译器
    Compiler* GetCompiler(const QString& playerName);

    /// 尝试获取内置校验器名称，不是内置校验器返回原始文件名
    QString BuiltinCheckerName() const
    {
        auto p = BUILTIN_CHECKER_MAP.find(RemoveFileExtension(checker));
        if (p != BUILTIN_CHECKER_MAP.end()) return p.value().first; else return checker;
    }

    /// InOutString，格式如下
    QString GetInOutString(TestCase *point) const
    {
        if (!point) return "";
        QString s = QString("标准输入:\"%1\" 标准输出:\"%2\"").arg(point->InFile()).arg(point->OutFile());
        if (type == Global::AnswersOnly) s += QString(" 选手提交:\"%1\"").arg(point->SubmitFile());
        return s;
    }

private:
    Global::ProblemType type;
    const QString name;
    QString dir, exe, checker, in_file, out_file;
    int score, checker_time_lim;
    double code_len_lim;

    QList<TestCase*> cases;
    QList<Subtask*> subtasks;
    QList<Compiler*> compilers;

    /// 自动获取输入输出文件对
    QList<QPair<QString, QString>> getInAndOutFile();
};

#endif // PROBLEM_H
