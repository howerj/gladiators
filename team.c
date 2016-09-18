#include "team.h"
#include "util.h"
#include <stdbool.h>

static bool warned = false;

color_t team_to_color(unsigned team)
{
	static const color_t colors[] = { RED, GREEN, YELLOW, CYAN, BLUE, MAGENTA };
	if(team >= sizeof(colors)/sizeof(colors[0])) {
		if(!warned) {
			warning("gladiator: ran out of team colors %u");
			warned = true;
		}
		return MAGENTA;
	}
	return colors[team];
}


