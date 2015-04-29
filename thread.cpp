#include "thread.h"
#include <iostream>

//-----------------------------------------------------------------------------
//
// Embedding python implementation
//
//-----------------------------------------------------------------------------
PyObject* Thread::globalDict     = 0;
PyObject* Thread::localDict      = 0;
float     Thread::camera_array[] = {0.0f, 0.0f, 5.0f};


void Thread::python_subscribe()
{
    //Py_SetProgramName("test"); /* optional but recommended */
//    Py_Initialize();
//    PyRun_SimpleString("import sys \n"
//                       "sys.path.append('/Users/haoluo/Downloads/python-master') \n"
//                       "from Pubnub import Pubnub \n"
//                       "pubnub = Pubnub(publish_key='pub-c-07cc1d84-8010-481f-99d0-812b2ce95dfe', subscribe_key='sub-c-bd087e9e-e899-11e4-9685-0619f8945a4f') \n"
//                       "def _callback(message, channel): \n"
//                       "    print 'Message received!' \n"
//                       "    global camera_pos \n"
//                       "    camera_pos = message[0] \n"
//                       "    print camera_pos \n"
//                       "def _error(message): \n"
//                       "    print('ERROR: ' + str(message)) \n"
//                       "def _subscribe(): \n"
//                       "    global i \n"
//                       "    pubnub.subscribe(channels='my_channel', callback=_callback, error=_error) \n"
//                       "    print 'my_channel subscribed!' \n"
//                       "_subscribe() \n");
//    Py_Finalize();

//    PyObject *pName, *pModule, *pDict, *pFunc;
//    PyObject *pArgs, *pValue;
//    std::string moduleName("receive");

//    Py_Initialize();
//    PyRun_SimpleString("import sys \n"
//                       "sys.path.append('/Users/haoluo/PycharmProjects/test') \n");

//    pName = PyString_FromString( moduleName.c_str() );
//    pModule = PyImport_Import(pName);
//    Py_DECREF(pName);

//    if(pModule != NULL){
//        pFunc = PyObject_GetAttrString(pModule, "_subscribe");
//        /* pFunc is a new reference */
//        if(pFunc && PyCallable_Check(pFunc) ){
//            //pArgs = PyTuple_New( 0 );
//            //std::cerr<<"Python callbale ok!"<<std::endl;
//            pValue = PyObject_CallObject(pFunc, NULL);
//            //PyObject_CallObject(pFunc, NULL);

//            if (pValue != NULL) {
//                std::cout<<"Result of call: "<< PyString_AsString( pValue )<<std::endl;
//                Py_DECREF(pValue);
//            }else{
//                Py_DECREF(pFunc);
//                Py_DECREF(pModule);
//                PyErr_Print();
//                fprintf(stderr, "Call failed\n");
//            }
//        }
//    }else{
//        PyErr_Print();
//        fprintf(stderr, "Failed to load \"%s\"\n", moduleName.c_str() );
//    }


    Py_Initialize();

//    PyRun_SimpleString("import sys \n"
//                       "sys.path.append('/Users/haoluo/PycharmProjects/test') \n");

    PyObject* main       = PyImport_AddModule("__main__");
//    PyObject* emb_Module = PyImport_ImportModule("receive");

    globalDict = PyModule_GetDict(main);
    localDict  = PyDict_New();

    //PyObject_SetAttrString(main, "receive", emb_Module);

    PyObject* value = PyInt_FromLong(0);
    PyDict_SetItemString(globalDict, "camera_pos", value);

    FILE* file;
    file = fopen("/Users/haoluo/PycharmProjects/test/receive.py", "r");
    PyRun_SimpleFile(file, "receive.py");// Py_single_input, globalDict, localDict);

    Py_Finalize();
}


void *Thread::python_retrieve_camera()
{
    static float old_camera_array[] = {-1, -1, -1};
    PyObject* camera_pos_list = PyDict_GetItemString( globalDict, "camera_pos" );

    if( PyList_Check(camera_pos_list) ){
        PyObject* camera_pos_tuple = PyList_AsTuple( camera_pos_list );

        if( PyTuple_Check(camera_pos_tuple) ){
            int tuple_size = PyTuple_Size( camera_pos_tuple );
            //camera_array = malloc( tuple_size * sizeof(float) );
            for (int i = 0; i < tuple_size; i++){
                PyObject* item = PyTuple_GetItem( camera_pos_tuple, i );
                //result = PyNumber_Check(item);
                //PyObject* temp_item = PyNumber_Long(item);
                camera_array[i] = PyFloat_AsDouble( item );
                Py_DECREF(item);

//                if( PyErr_Occurred() ){
                   // qDebug()<<"py error";
//                    camera_array[0] = old_camera_array[0];
//                    camera_array[1] = old_camera_array[1];
//                    camera_array[2] = old_camera_array[2];
 //               }
            }
        }
//        if(PyErr_Occurred())
//            qDebug()<<"py error";
    }

    if( camera_array[0] != old_camera_array[0] ||
        camera_array[1] != old_camera_array[1] ||
        camera_array[2] != old_camera_array[2] ){

        qDebug()<<"camera:"<<camera_array[0]
                           <<camera_array[1]
                           <<camera_array[2];

        old_camera_array[0] = camera_array[0];
        old_camera_array[1] = camera_array[1];
        old_camera_array[2] = camera_array[2];
    }

    //return camera_array;
}

//-----------------------------------------------------------------------------
//
// Embedding python implementation
//
//-----------------------------------------------------------------------------
//void* show_window(void* thread_nr)
//{
//    MainWindow w;
//    w.show();
//    return 0;
//}

//void multi_thread( std::vector< void *(*)(void *) > my_funcs )
//{
//    pthread_t t1, t2;
//    static int current_t = 0;
//    std::cout << "Launching thread " << current_t <<"..." << std::endl;
//    int temp_arg = current_t;
//    int result = pthread_create(&t1, NULL, my_funcs[0], static_cast<void*>(&temp_arg) );
//    temp_arg = current_t + 1;
//    result = pthread_create(&t2, NULL, my_funcs[1], static_cast<void*>(&temp_arg) );
//    if (result !=0){
//        std::cerr << "Error creating thread " << current_t << ". Return code:" << result <<  std::endl;
//    }
//    pthread_join(t1, NULL);
//    pthread_join(t2, NULL);
//}

//    std::vector< void *(*)(void *) > my_funcs;
//    my_funcs.push_back( show_window );
//    my_funcs.push_back( python_run );
//    multi_thread( my_funcs );
