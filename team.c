#include "team.h"

color_t team_to_color(unsigned team)
{
	static const color_t colors[] = { RED, GREEN, YELLOW, CYAN, BLUE, MAGENTA };
	if(team >= INVALID_COLOR) {
		warning("gladiator: ran out of team colors %u");
		return MAGENTA;
	}
	return colors[team];
}


