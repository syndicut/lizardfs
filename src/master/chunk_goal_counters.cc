#include "common/platform.h"
#include "master/chunk_goal_counters.h"

#include "common/goal.h"

ChunkGoalCounters::ChunkGoalCounters() : fileCount_(0), goal_(0) {}

void ChunkGoalCounters::addFile(uint8_t goal) {
	if (!isGoalValid(goal)) {
		throw InvalidOperation("Invalid goal " + std::to_string(goal));
	}
	if (fileCount_ == 0) {
		goal_ = goal;
		fileCount_ = 1;
		return;
	}
	if (!fileCounters_ && goal != goal_) {
		fileCounters_.reset(new GoalMap<uint32_t>());
		(*fileCounters_)[goal_] = fileCount_;
	}
	fileCount_++;
	if (fileCounters_) {
		(*fileCounters_)[goal]++;
	}
	goal_ = calculateGoal();
}

void ChunkGoalCounters::removeFile(uint8_t goal) {
	removeFileInternal(goal);
	goal_ = calculateGoal();
	tryDeleteFileCounters();
}

void ChunkGoalCounters::changeFileGoal(uint8_t prevGoal, uint8_t newGoal) {
	removeFileInternal(prevGoal);
	addFile(newGoal);
	tryDeleteFileCounters();
}

uint8_t ChunkGoalCounters::calculateGoal() {
	if (fileCount_ == 0) {
		sassert(!fileCounters_);
		// No files - no goal
		return 0;
	} else if (fileCounters_) {
		for (uint8_t goal = kMaxGoal; goal >= kMinGoal; --goal) {
			// Effective goal is the highest used one
			if ((*fileCounters_)[goal] != 0) {
				return goal;
			}
		}
	} else {
		return goal_;
	}
	throw InvalidOperation("This should never happen");
}

void ChunkGoalCounters::tryDeleteFileCounters() {
	if (!fileCounters_) {
		return;
	}

	uint8_t goalsUsed = 0;
	for (uint8_t iGoal = kMaxGoal; iGoal >= kMinGoal; --iGoal) {
		if ((*fileCounters_)[iGoal] > 0) {
			++goalsUsed;
		}
	}
	// Found only one goal used? Optimise!
	if (goalsUsed == 1) {
		fileCounters_.reset();
	}
}

void ChunkGoalCounters::removeFileInternal(uint8_t goal) {
	if (!isGoalValid(goal)) {
		throw InvalidOperation("Invalid goal " + std::to_string(goal));
	}
	if (fileCounters_) {
		sassert(fileCount_ > 1);
		if ((*fileCounters_)[goal] == 0) {
			throw InvalidOperation("No file with goal " + std::to_string(goal) + " to remove");
		}
		(*fileCounters_)[goal]--;
	} else {
		if (goal_ != goal) {
			throw InvalidOperation("No file with goal " + std::to_string(goal) + " to remove");
		}
	}
	fileCount_--;
}
