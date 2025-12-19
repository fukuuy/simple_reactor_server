#include"Server.h"
#include"Handler.h"
#include"ThreadPool.h"

int main()
{
	Server server("192.168.80.128", 10000);
    Reactor reactor;
    ThreadPool threadpool(3,100);

    server.AddReactor(&reactor, Handler::handleAccpet);
    server.start();
    //cout << "server fd status: " << reactor.findevent(server.get_fd()).status() << endl;
    reactor.AddThreadPool(&threadpool);
    reactor.loop();

	return 0;
}