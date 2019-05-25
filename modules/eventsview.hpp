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
	struct Event
	{
		std::string title;
		std::time_t start, end;
		std::string room;
		bool is_series;
	};

	void init() override;

	void render() override;

	std::optional<Event> current_event() const;
};

#endif // EVENTSVIEW_HPP
