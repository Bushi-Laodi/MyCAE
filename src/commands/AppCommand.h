#pragma once

class AppCommand
{
public:
    virtual ~AppCommand() = default;
    virtual void execute() = 0;
};
