//QT_MANGLE_NAMESPACE
#ifdef QT_MANGLE_NAMESPACE
	#undef QT_MANGLE_NAMESPACE
#endif
//qt_resource_data
#ifdef qt_resource_data
	#undef qt_resource_data
#endif
//qt_resource_name
#ifdef qt_resource_name
	#undef qt_resource_name
#endif
//qt_resource_struct
#ifdef qt_resource_struct
#undef qt_resource_struct
#endif

#if QNUM==1
	#define QT_MANGLE_NAMESPACE(name) name##_##1
	#define qt_resource_data qt_resource_data_##1
	#define qt_resource_name qt_resource_name_##1
	#define qt_resource_struct qt_resource_struct_##1
#elif QNUM==2
	#define QT_MANGLE_NAMESPACE(name) name##_##2
	#define qt_resource_data qt_resource_data_##2
	#define qt_resource_name qt_resource_name_##2
	#define qt_resource_struct qt_resource_struct_##2
#elif QNUM==3
	#define QT_MANGLE_NAMESPACE(name) name##_##3
	#define qt_resource_data qt_resource_data_##3
	#define qt_resource_name qt_resource_name_##3
	#define qt_resource_struct qt_resource_struct_##3
#elif QNUM==4
	#define QT_MANGLE_NAMESPACE(name) name##_##4
	#define qt_resource_data qt_resource_data_##4
	#define qt_resource_name qt_resource_name_##4
	#define qt_resource_struct qt_resource_struct_##4
#elif QNUM==5
	#define QT_MANGLE_NAMESPACE(name) name##_##5
	#define qt_resource_data qt_resource_data_##5
	#define qt_resource_name qt_resource_name_##5
	#define qt_resource_struct qt_resource_struct_##5
#endif
