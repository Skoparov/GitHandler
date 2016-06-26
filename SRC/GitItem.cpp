#include "GitItem.h"

namespace git_handler
{

namespace item
{

enum class Type
{
	GIT_REPO,
	GIT_REMOTE,
	GIT_COMMIT,
	GIT_REF,
	GIT_STR_ARR,
	GIT_REV_WALK
};

} //item

} // git_handler
