#pragma once

struct Clocked
{
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void tick() = 0;
};
