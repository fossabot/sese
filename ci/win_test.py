import ci.common.test as test
from ci.common.config import BuildType

test.test_with('debug', BuildType.RELEASE)
