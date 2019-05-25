#ifndef EVENTSVIEW_HPP
#define EVENTSVIEW_HPP

#include "gui_module.hpp"

//!
//! Displays events that happen in the next
//! week in shackspace.
//!
//! Queries from https://events.shackspace.de/
//!
struct eventsview : gui_module
{
	void init() override;

	void render() override;
};

#endif // EVENTSVIEW_HPP
