#include <QFile>
#include <QElapsedTimer>
#include <QCoreApplication>

#include "common/global.h"
#include "common/player.h"
#include "common/problem.h"
#include "judge/judgethread.h"
#include "judge/judger/answeronlyjudger.h"
#include "judge/judger/traditionaljudger.h"

bool JudgeThread::is_judging = false;

JudgeThread::JudgeThread(int row, int column, QObject* parent) : QThread(parent),
    row(row), column(column)
{

}

bool JudgeThread::WaitForFinished(int ms)
{
    QElapsedTimer timer;
    for (timer.start(); this->isRunning() && timer.elapsed() <= ms;)
    {
        QCoreApplication::processEvents();
        QThread::msleep(10);
    }
    return this->isFinished();
}

void JudgeThread::cleanResultLabel(Player* player, int column)
{
    ResultLabel* tmp = player->LabelAt(column);
    player->SumLabel()->Subtract(tmp->Result());
    tmp->SetResult(0, 0, ' ');

    emit labelTextChanged(tmp, "", "未测评", Global::StyleNone);
    emit sumLabelChanged(player);

    QFile(Global::g_contest.result_path + Global::g_contest.problems[column - 2]->Name() + "/" + player->Name() + ".res").remove();
}

void JudgeThread::judgeProblem(Player* player, int column)
{
    ResultLabel* tmp = player->LabelAt(column);

    emit labelTextChanged(tmp, "~", "正在测评...", Global::StyleRunning);

    const Problem* prob = Global::g_contest.problems[column - 2];
    BaseJudger* judger;
    if (prob->Type() == Global::AnswersOnly)
        judger = new AnswerOnlyJudger(Global::g_contest.path, player, prob);
    else
        judger = new TraditionalJudger(Global::g_contest.path, player, prob);

    connect(this, &JudgeThread::judgeStoped, judger, &BaseJudger::StopJudge);
    connect(judger, &BaseJudger::titleDetailFinished, this, &JudgeThread::titleDetailFinished);
    connect(judger, &BaseJudger::noteDetailFinished,  this, &JudgeThread::noteDetailFinished);
    connect(judger, &BaseJudger::pointDetailFinished, this, &JudgeThread::pointDetailFinished);
    connect(judger, &BaseJudger::scoreDetailFinished, this, &JudgeThread::scoreDetailFinished);

    ResultSummary res = judger->Judge();
    delete judger;

    tmp->SetResult(res);
    player->SumLabel()->Plus(res);

    emit problemLabelChanged(player, column);
    emit sumLabelChanged(player);

    QCoreApplication::processEvents();
    msleep(20);

    player->SaveHTMLResult();
}

void JudgeThread::run()
{
    is_judging = true;

    if (0 <= column && column <= 1) // Judge one player's all problems
    {
        int t = Global::GetLogicalRow(row);
        Player* player = Global::g_contest.players[t];
        for (auto i : Global::g_contest.problem_order) cleanResultLabel(player, i + 2);
        for (auto i : Global::g_contest.problem_order)
        {
            judgeProblem(player, i + 2);
            if (Global::g_is_judge_stoped) break;
            emit itemJudgeFinished(row, i + 2);
        }
    }
    else // Judge tasks in judgeList
    {
        for (auto i : judge_list)
        {
            int t = Global::GetLogicalRow(i.first);
            Player* player = Global::g_contest.players[t];
            cleanResultLabel(player, i.second);
            judgeProblem(player, i.second);
            if (Global::g_is_judge_stoped) break;
            emit itemJudgeFinished(i.first, i.second);
        }
    }

    emit noteDetailFinished(Global::g_is_judge_stoped ? "- 测评终止 -" : "- 测评结束 -", "");
    is_judging = false;
}
