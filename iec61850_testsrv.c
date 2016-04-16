/*
 * test de serveur iec61850
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <libiec61850/iec61850_server.h>
#include <libiec61850/hal_thread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

static int running = 0;
static IedServer iedServer = NULL;
DataAttribute *varSAV_setMagF, *varSAV_t ;

void sigint_handler(int signalId)
{
	running = 0;
}

int launchIedServer(int port_61850)
{
	int i;
	time_t currTime;
	currTime=time(NULL);
	printf("Starting IED server %s", ctime(&currTime));
	IedModel* myModel = IedModel_create("test");
	LogicalDevice* lDevice1 = LogicalDevice_create("Device", myModel);
	LogicalNode* lln0 = LogicalNode_create("LLN0", lDevice1);
	char buffer[16];
	CDC_ASG_create("varASG", (ModelNode*) lln0, 0, false);
	DataObject * varSAV = CDC_SAV_create("varSAV", (ModelNode*) lln0, 0, false);
	varSAV_setMagF = (DataAttribute*) ModelNode_getChild((ModelNode*) varSAV, "instMag.f");
	varSAV_t = (DataAttribute*) ModelNode_getChild((ModelNode*) varSAV, "t");
	iedServer = IedServer_create(myModel);

	IedServer_start(iedServer, port_61850);
	if (!IedServer_isRunning(iedServer)) {
		fprintf(stderr, "Starting IED server failed! Exit.\n");
		IedServer_destroy(iedServer);
		exit(-1);
	}
	printf("Iec61850 test server is ready\n");
	return 1;
}

int main(int argc, char **argv)
{
	int port_61850 = 102;
	int i;
	if (argc > 1)
		port_61850 = atoi(argv[1]);
	running = 1;
	signal(SIGINT, sigint_handler);
	launchIedServer(port_61850);
	float myfloat;
	while (running) 
	{
		sleep(1);
		uint64_t timeval = Hal_getTimeInMs();
		IedServer_lockDataModel(iedServer);
		IedServer_updateUTCTimeAttributeValue(iedServer, varSAV_t, timeval);
		IedServer_updateFloatAttributeValue(iedServer, varSAV_setMagF, myfloat);
		IedServer_unlockDataModel(iedServer);
		myfloat+=1e-3;
	}

	printf("Stoping iec server\n");
	IedServer_stop(iedServer);
	IedServer_destroy(iedServer);

	return (0);
}
