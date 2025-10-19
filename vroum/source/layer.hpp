#pragma once

class Application;

namespace vv
{

class Layer
{
public:
	virtual ~Layer() {}

	virtual void render() = 0;

	virtual void update() = 0;

private:
	Application *m_app;
};

} // namespace vv