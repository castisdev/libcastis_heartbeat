# SVM Heartbeat Responser

`ServiceMonitor`의 Heartbeat 요청에 응답하기위한 연동 API

## Example
```cpp
#include "CiHBResponser.h"

int main()
{
	CCiHBResponser hbr("127.0.0.1", 8090, "127.0.0.1", 10);
	hbr.RunThreadHere();

	return 0;
}

```

## Dependency

* legacy_compatible
    * None

* legacy_compatible_with_ciutils
    * CiUtils
    * Boost >= 1.33.1
        * system

* legacy_compatible_complex_with_ciutils
    * CiUtils
    * Boost >= 1.33.1
        * system
