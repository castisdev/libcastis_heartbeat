# SVM Heartbeat Responser

**ServiceMonitor**의 Heartbeat 요청에 응답하기 위한 연동 라이브러리

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
