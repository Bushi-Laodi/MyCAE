/*
Copyright (c) 2020-2026, Qingdao Digital Intelligent Ship & Ocean Technology Co., Ltd.
All rights reserved.

This file is part of FastCAE and is distributed under the terms of the
BSD 3-Clause License. See the LICENSE file in the project root for details.
*/

/**********************************************************************
 * @file   CalculateDriver.h
 * @brief  求解器第三方程序驱动类
 * @author BaGuijun (baguijun@163.com)
 * @date   2024-09-05
 *********************************************************************/
#include "FITK_Kernel/FITKAppFramework/FITKExecProgramDriver.h"
#include "FITK_Kernel/FITKAppFramework/FITKProgramDriverFactory.h"

#include <QProcess>

namespace GUI
{
    class CalculateDriver : public AppFrame::FITKAbstractProgramerDriver
    {
        Q_OBJECT;
    public:
        CalculateDriver();
        ~CalculateDriver();
        /**
         * @brief  设置可执行程序的名称
         * @param[i]  program       程序的路径和名称
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        void setExecProgram(const QString & program);
        /**
         * @brief  获取可执行程序的名称
         * @return QString      程序路径和名称
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        QString getExecProgram() const;
        virtual int getProgramType() override;
        virtual QString getProgramName() override;

        /**
        * @brief 启动程序
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
        */
        void start() override;
        /**
         * @brief 终止程序运行
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        void stop() override;
    private slots:
        ;
        void slotProcessOutput();
        void slotProcessOutputError();
        void slotProcessError(QProcess::ProcessError error);
        void slotProcessFinish(int exitCode, QProcess::ExitStatus exitStatus);
        void slotMainwindowClose();
    private:
        /**
         * @brief 可执行程序名称与路径
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        QString _program{};
        /**
         * @brief 进程对象
         * @author BaGuijun (baguijun@163.com)
         * @date   2024-09-05
         */
        QProcess* _process{};
    };

    Register2FITKProgramDriverFactory(1, CalculateDriver, CalculateDriver)
} // namespace GUI

    