#pragma once

struct InputState
{
	bool left;
	bool right;
	bool jumpPressed;
	bool attackPressed;
	bool rollPressed;
	bool sprintHeld;
	bool specialPressed;
	bool normalAttacks;
	bool elixirPressed;

	bool attackJustPressed = false;
	bool jumpJustPressed = false;
	bool specialJustPressed = false;
};
