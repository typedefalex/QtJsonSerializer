#include <QtTest>
#include <QtJsonSerializer>

#include "testconverter.h"

using TestTuple = std::tuple<int, QString, QList<int>>;
using TestPair = std::pair<bool, int>;
using TestVariant = std::variant<bool, int, double>;
Q_DECLARE_METATYPE(TestTuple)
Q_DECLARE_METATYPE(TestPair)
Q_DECLARE_METATYPE(std::optional<int>)
Q_DECLARE_METATYPE(TestVariant)

class AliasHelper : public QJsonSerializerBase
{
public:
	QByteArray getNameHelper(int propertyType) const {
		return getCanonicalTypeName(propertyType);
	}

protected:
	bool jsonMode() const override {
		return true;
	}
	QCborTag typeTag(int metaTypeId) const override {
		return static_cast<QCborTag>(-1);
	}
	QList<int> typesForTag(QCborTag tag) const override {
		return {};
	}
};

class SerializerTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testAliasName();
	void testVariantConversions_data();
	void testVariantConversions();

	void testSerialization_data();
	void testSerialization();
	void testDeserialization_data();
	void testDeserialization();

//	void testDeviceSerialization();
//	void testExceptionTrace();

private:
	AliasHelper *helper;
	QJsonSerializer *serializer = nullptr;

	void addCommonData();
	void resetProps();
};

void SerializerTest::initTestCase()
{
	//aliases
	qRegisterMetaType<IntAlias>("IntAlias");
	qRegisterMetaType<ListAlias>();
	QJsonSerializer::registerInverseTypedef<ListAlias>("QList<TestObject*>");

	// converters
	QJsonSerializer::registerPointerConverters<TestObject>();
	QJsonSerializer::registerListConverters<TestObject*>();
	QJsonSerializer::registerListConverters<QList<int>>();
	QJsonSerializer::registerMapConverters<TestObject*>();
	QJsonSerializer::registerMapConverters<QMap<QString, int>>();
	QJsonSerializer::registerPairConverters<int, QString>();
	QJsonSerializer::registerPairConverters<bool, bool>();
	QJsonSerializer::registerPairConverters<QList<bool>, bool>();
	QJsonSerializer::registerListConverters<QPair<bool, bool>>();

	QJsonSerializer_registerTupleConverters_named(int, QString, QList<int>);
	QJsonSerializer_registerStdPairConverters_named(bool, int);
	QJsonSerializer::registerOptionalConverters<int>();
	QJsonSerializer_registerVariantConverters_named(bool, int, double);

	//register list comparators, needed for test only!
	QMetaType::registerEqualsComparator<QList<int>>();
	QMetaType::registerEqualsComparator<QList<TestObject*>>();
	QMetaType::registerEqualsComparator<QList<QList<int>>>();
	QMetaType::registerEqualsComparator<QMap<QString, int>>();
	QMetaType::registerEqualsComparator<QMap<QString, TestObject*>>();
	QMetaType::registerEqualsComparator<QMap<QString, QMap<QString, int>>>();
	QMetaType::registerEqualsComparator<QPair<QVariant, QVariant>>();
	QMetaType::registerEqualsComparator<QPair<int, QString>>();
	QMetaType::registerEqualsComparator<QPair<QList<bool>, bool>>();
	QMetaType::registerEqualsComparator<QList<QPair<bool, bool>>>();

	QMetaType::registerEqualsComparator<TestTuple>();
	QMetaType::registerEqualsComparator<TestPair>();
	QMetaType::registerEqualsComparator<std::optional<int>>();
	QMetaType::registerEqualsComparator<std::variant<bool, int, double>>();

	helper = new AliasHelper{};
	serializer = new QJsonSerializer{this};
	serializer->addJsonTypeConverter<TestEnumConverter>();
	serializer->addJsonTypeConverter<TestWrapperConverter>();
}

void SerializerTest::cleanupTestCase()
{
	delete serializer;
	serializer = nullptr;
}

void SerializerTest::testAliasName()
{
	QCOMPARE(QMetaType::typeName(qMetaTypeId<ListAlias>()), "ListAlias");
	QCOMPARE(helper->getNameHelper(qMetaTypeId<ListAlias>()), "QList<TestObject*>");
}

void SerializerTest::testVariantConversions_data()
{
	QTest::addColumn<QVariant>("data");
	QTest::addColumn<int>("targetType");
	QTest::addColumn<QVariant>("variantData");

	QTest::newRow("QList<int>") << QVariant::fromValue<QList<int>>({3, 7, 13})
								<< static_cast<int>(QMetaType::QVariantList)
								<< QVariant{QVariantList{3, 7, 13}};
	auto to = new TestObject{this};
	QTest::newRow("QList<TestObject*>") << QVariant::fromValue<QList<TestObject*>>({nullptr, to, nullptr})
										<< static_cast<int>(QMetaType::QVariantList)
										<< QVariant{QVariantList{
											   QVariant::fromValue<TestObject*>(nullptr),
											   QVariant::fromValue(to),
											   QVariant::fromValue<TestObject*>(nullptr)
										   }};
	QList<int> l1 = {0, 1, 2};
	QList<int> l2 = {3, 4, 5};
	QList<int> l3 = {6, 7, 8};
	QTest::newRow("QList<QList<int>>") << QVariant::fromValue<QList<QList<int>>>({l1, l2, l3})
									   << static_cast<int>(QMetaType::QVariantList)
									   << QVariant{};
	QTest::newRow("QStringList") << QVariant::fromValue<QStringList>({QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")})
								 << static_cast<int>(QMetaType::QVariantList)
								 << QVariant{QVariantList{QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c")}};

	QTest::newRow("QMap<QString, int>") << QVariant::fromValue<QMap<QString, int>>({
																					   {QStringLiteral("baum"), 42},
																					   {QStringLiteral("devil"), 666},
																					   {QStringLiteral("fun"), 0}
																				   })
										<< static_cast<int>(QMetaType::QVariantMap)
										<< QVariant{QVariantMap {
												{QStringLiteral("baum"), 42},
												{QStringLiteral("devil"), 666},
												{QStringLiteral("fun"), 0}
											}};
	QTest::newRow("QMap<QString, TestObject*>") << QVariant::fromValue<QMap<QString, TestObject*>>({
													   {QStringLiteral("baum"), nullptr},
													   {QStringLiteral("devil"), to},
													   {QStringLiteral("fun"), nullptr}
												   })
												<< static_cast<int>(QMetaType::QVariantMap)
												<< QVariant{QVariantMap {
													   {QStringLiteral("baum"), QVariant::fromValue<TestObject*>(nullptr)},
													   {QStringLiteral("devil"), QVariant::fromValue(to)},
													   {QStringLiteral("fun"), QVariant::fromValue<TestObject*>(nullptr)}
												   }};
	QMap<QString, int> m1 = {{QStringLiteral("v0"), 0}, {QStringLiteral("v1"), 1}, {QStringLiteral("v2"), 2}};
	QMap<QString, int> m2 = {{QStringLiteral("v3"), 3}, {QStringLiteral("v4"), 4}, {QStringLiteral("v5"), 5}};
	QMap<QString, int> m3 = {{QStringLiteral("v6"), 6}, {QStringLiteral("v7"), 7}, {QStringLiteral("v8"), 8}};
	QTest::newRow("QMap<QString, QMap<QString, int>>") << QVariant::fromValue<QMap<QString, QMap<QString, int>>>({
																													 {QStringLiteral("m1"), m1},
																													 {QStringLiteral("m2"), m2},
																													 {QStringLiteral("m3"), m3}
																												 })
													   << static_cast<int>(QMetaType::QVariantMap)
													   << QVariant{};

	QSharedPointer<TestObject> sPtr(new TestObject{});
	QTest::newRow("QSharedPointer<TestObject>") << QVariant::fromValue(sPtr)
												<< qMetaTypeId<QSharedPointer<QObject>>()
												<< QVariant::fromValue(sPtr.staticCast<QObject>());
	QPointer<TestObject> oPtr(new TestObject{this});
	QTest::newRow("QPointer<TestObject>") << QVariant::fromValue(oPtr)
										  << qMetaTypeId<QPointer<QObject>>()
										  << QVariant::fromValue<QPointer<QObject>>(oPtr.data());

	QTest::newRow("QPair<int, QString>") << QVariant::fromValue<QPair<int, QString>>({42, QStringLiteral("baum")})
										 << qMetaTypeId<QPair<QVariant, QVariant>>()
										 << QVariant::fromValue(QPair<QVariant, QVariant>{42, QStringLiteral("baum")});

	QTest::newRow("QPair<QList<bool>, bool>") << QVariant::fromValue<QPair<QList<bool>, bool>>({{true, false, true}, false})
											  << qMetaTypeId<QPair<QVariant, QVariant>>()
											  << QVariant{};

	QTest::newRow("QList<QPair<bool, bool>>") << QVariant::fromValue<QList<QPair<bool, bool>>>({{false, true}, {true, false}})
											  << static_cast<int>(QMetaType::QVariantList)
											  << QVariant{};

	QTest::newRow("std::tuple<int, QString, QList<int>>") << QVariant::fromValue<TestTuple>(std::make_tuple(42, QStringLiteral("Hello World"), QList<int>{1, 2, 3}))
														  << static_cast<int>(QMetaType::QVariantList)
														  << QVariant{QVariantList{42, QStringLiteral("Hello World"), QVariant::fromValue(QList<int>{1, 2, 3})}};
	QTest::newRow("std::pair<bool, int>") << QVariant::fromValue<TestPair>(std::make_pair(true, 20))
										  << qMetaTypeId<QPair<QVariant, QVariant>>()
										  << QVariant::fromValue(QPair<QVariant, QVariant>{true, 20});

	QVariant optv{42};
	QTest::newRow("std::optional<int>") << QVariant::fromValue<std::optional<int>>(42)
										<< static_cast<int>(QMetaType::QVariant)
										<< QVariant{QMetaType::QVariant, &optv};
	optv = QVariant::fromValue(nullptr);
	QTest::newRow("std::nullopt") << QVariant::fromValue<std::optional<int>>(std::nullopt)
								  << static_cast<int>(QMetaType::QVariant)
								  << QVariant{QMetaType::QVariant, &optv};

	QVariant varv1{true};
	QTest::newRow("std::variant<bool>") << QVariant::fromValue<TestVariant>(true)
										<< static_cast<int>(QMetaType::QVariant)
										<< QVariant{QMetaType::QVariant, &varv1};
	QVariant varv2{42};
	QTest::newRow("std::variant<int>") << QVariant::fromValue<TestVariant>(42)
									   << static_cast<int>(QMetaType::QVariant)
									   << QVariant{QMetaType::QVariant, &varv2};
	QVariant varv3{4.2};
	QTest::newRow("std::variant<double>") << QVariant::fromValue<TestVariant>(4.2)
										  << qMetaTypeId<QVariant>()
										  << QVariant{QMetaType::QVariant, &varv3};
}

void SerializerTest::testVariantConversions()
{
	QFETCH(QVariant, data);
	QFETCH(int, targetType);
	QFETCH(QVariant, variantData);

	auto origType = data.userType();
	auto convData = data;
	QVERIFY(convData.convert(targetType));
	if (variantData.isValid())
		QCOMPARE(convData, variantData);
	QVERIFY(convData.convert(origType));
	QCOMPARE(convData, data);
}

void SerializerTest::testSerialization_data()
{
	QTest::addColumn<QVariant>("data");
	QTest::addColumn<QJsonValue>("result");
	QTest::addColumn<bool>("works");
	QTest::addColumn<QVariantHash>("extraProps");

	addCommonData();
}

void SerializerTest::testSerialization()
{
	QFETCH(QVariant, data);
	QFETCH(QJsonValue, result);
	QFETCH(bool, works);
	QFETCH(QVariantHash, extraProps);

	resetProps();
	for(auto it = extraProps.constBegin(); it != extraProps.constEnd(); it++)
		serializer->setProperty(qUtf8Printable(it.key()), it.value());

	try {
		if(works) {
			auto res = serializer->serialize(data);
			QCOMPARE(res, result);
		} else
			QVERIFY_EXCEPTION_THROWN(serializer->serialize(data), QJsonSerializationException);
	} catch(std::exception &e) {
		QFAIL(e.what());
	}
}

void SerializerTest::testDeserialization_data()
{
	QTest::addColumn<QVariant>("result");
	QTest::addColumn<QJsonValue>("data");
	QTest::addColumn<bool>("works");
	QTest::addColumn<QVariantHash>("extraProps");

	addCommonData();

	QTest::newRow("null.invalid.bool") << QVariant{false}
									   << QJsonValue{QJsonValue::Null}
									   << false
									   << QVariantHash{};
	QTest::newRow("null.invalid.int") << QVariant{0}
									  << QJsonValue{QJsonValue::Null}
									  << false
									  << QVariantHash{};
	QTest::newRow("null.invalid.double") << QVariant{0.0}
										 << QJsonValue{QJsonValue::Null}
										 << false
										 << QVariantHash{};
	QTest::newRow("null.invalid.string") << QVariant{QString()}
										 << QJsonValue{QJsonValue::Null}
										 << false
										 << QVariantHash{};
	QTest::newRow("null.invalid.uuid") << QVariant{QUuid()}
									   << QJsonValue{QJsonValue::Null}
									   << false
									   << QVariantHash{};
	QTest::newRow("null.invalid.url") << QVariant{QUrl()}
									  << QJsonValue{QJsonValue::Null}
									  << false
									  << QVariantHash{};
	QTest::newRow("null.invalid.regex") << QVariant{QRegularExpression()}
										<< QJsonValue{QJsonValue::Null}
										<< false
										<< QVariantHash{};

	QTest::newRow("null.valid.bool") << QVariant{false}
									 << QJsonValue{QJsonValue::Null}
									 << true
									 << QVariantHash {
											{QStringLiteral("allowDefaultNull"), true},
											{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										};
	QTest::newRow("null.valid.int") << QVariant{0}
									<< QJsonValue{QJsonValue::Null}
									<< true
									<< QVariantHash {
										   {QStringLiteral("allowDefaultNull"), true},
										   {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
									   };
	QTest::newRow("null.valid.double") << QVariant{0.0}
									   << QJsonValue{QJsonValue::Null}
									   << true
									   << QVariantHash {
											  {QStringLiteral("allowDefaultNull"), true},
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.valid.string") << QVariant{QString()}
									   << QJsonValue{QJsonValue::Null}
									   << true
									   << QVariantHash {
											  {QStringLiteral("allowDefaultNull"), true},
											  {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										  };
	QTest::newRow("null.valid.uuid") << QVariant{QUuid()}
									 << QJsonValue{QJsonValue::Null}
									 << true
									 << QVariantHash {
											{QStringLiteral("allowDefaultNull"), true},
											{QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										};
	QTest::newRow("null.valid.url") << QVariant{QUrl()}
									<< QJsonValue{QJsonValue::Null}
									<< true
									<< QVariantHash {
										   {QStringLiteral("allowDefaultNull"), true},
										   {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
									   };
	QTest::newRow("null.valid.regex") << QVariant{QRegularExpression()}
									  << QJsonValue{QJsonValue::Null}
									  << true
									  << QVariantHash {
											 {QStringLiteral("allowDefaultNull"), true},
											 {QStringLiteral("validationFlags"), QVariant::fromValue<QJsonSerializer::ValidationFlags>(QJsonSerializer::ValidationFlag::StandardValidation)}
										 };

	QTest::newRow("strict.bool.invalid") << QVariant{false}
										 << QJsonValue{0}
										 << false
										 << QVariantHash{};
	QTest::newRow("strict.int.double") << QVariant{4}
									   << QJsonValue{4.2}
									   << false
									   << QVariantHash{};
	QTest::newRow("strict.int.type") << QVariant{2}
									 << QJsonValue{QStringLiteral("2")}
									 << false
									 << QVariantHash{};
	QTest::newRow("strict.string") << QVariant{QStringLiteral("10")}
								   << QJsonValue{10}
								   << false
								   << QVariantHash{};
	QTest::newRow("strict.nullptr") << QVariant::fromValue(nullptr)
									<< QJsonValue{QJsonObject{}}
									<< false
									<< QVariantHash{};
	QTest::newRow("strict.double") << QVariant{2.2}
								   << QJsonValue{QStringLiteral("2.2")}
								   << false
								   << QVariantHash{};
	QTest::newRow("strict.url") << QVariant{QUrl{}}
								<< QJsonValue{5}
								<< false
								<< QVariantHash{};
	QTest::newRow("strict.uuid") << QVariant{QUuid{}}
								 << QJsonValue{true}
								 << false
								 << QVariantHash{};
	QTest::newRow("strict.regex") << QVariant{QRegularExpression{}}
								  << QJsonValue{42}
								  << false
								  << QVariantHash{};
}

void SerializerTest::testDeserialization()
{
	QFETCH(QJsonValue, data);
	QFETCH(QVariant, result);
	QFETCH(bool, works);
	QFETCH(QVariantHash, extraProps);

	resetProps();
	for(auto it = extraProps.constBegin(); it != extraProps.constEnd(); it++)
		serializer->setProperty(qUtf8Printable(it.key()), it.value());

	try {
		if(works) {
			auto res = serializer->deserialize(data, result.userType(), this);
			QCOMPARE(res, result);
		} else
			QVERIFY_EXCEPTION_THROWN(serializer->deserialize(data, result.userType(), this), QJsonDeserializationException);
	} catch(std::exception &e) {
		QFAIL(e.what());
	}
}

//void SerializerTest::testDeviceSerialization()
//{
//	const TestGadget g{10};
//	const QByteArray bRes{R"__({"data":10})__"};

//	// to bytearray
//	auto ba = serializer->serializeTo(g, QJsonDocument::Compact);
//	QCOMPARE(ba, bRes);
//	auto gRes = serializer->deserializeFrom<TestGadget>(ba);
//	QCOMPARE(gRes, g);

//	//to device
//	ba.clear();
//	QBuffer buffer{&ba};
//	QVERIFY(buffer.open(QIODevice::WriteOnly | QIODevice::Text));
//	serializer->serializeTo(&buffer, g, QJsonDocument::Compact);
//	buffer.close();
//	QCOMPARE(ba, bRes);
//	QVERIFY(buffer.open(QIODevice::ReadOnly | QIODevice::Text));
//	gRes = serializer->deserializeFrom<TestGadget>(&buffer);
//	buffer.close();
//	QCOMPARE(gRes, g);

//	//invalid
//	QVERIFY_EXCEPTION_THROWN(serializer->serializeTo(42), QJsonSerializationException);
//}

//void SerializerTest::testExceptionTrace()
//{
//	try {
//		serializer->deserialize<QList<TestGadget>>({
//													   QJsonObject{
//														   {QStringLiteral("data"), QStringLiteral("test")}
//													   }
//												   });
//		QFAIL("No exception thrown");
//	} catch (QJsonSerializerException &e) {
//		auto trace = e.propertyTrace();
//		QCOMPARE(trace.size(), 2);
//		QCOMPARE(trace[0].first, QByteArray{"[0]"});
//		QCOMPARE(trace[0].second, QByteArray{"TestGadget"});
//		QCOMPARE(trace[1].first, QByteArray{"data"});
//		QCOMPARE(trace[1].second, QByteArray{"int"});
//	}
//}

void SerializerTest::addCommonData()
{
	// basic types without any converter
	QTest::newRow("bool") << QVariant{true}
						  << QJsonValue{true}
						  << true
						  << QVariantHash{};
	QTest::newRow("int") << QVariant{42}
						 << QJsonValue{42}
						 << true
						 << QVariantHash{};
	QTest::newRow("double") << QVariant{4.2}
							<< QJsonValue{4.2}
							<< true
							<< QVariantHash{};
	QTest::newRow("string.normal") << QVariant{QStringLiteral("baum")}
								   << QJsonValue{QStringLiteral("baum")}
								   << true
								   << QVariantHash{};
	QTest::newRow("string.empty") << QVariant{QString()}
								  << QJsonValue{QString()}
								  << true
								  << QVariantHash{};
	QTest::newRow("nullptr") << QVariant::fromValue(nullptr)
							 << QJsonValue{QJsonValue::Null}
							 << true
							 << QVariantHash{};

	// advanced types
	auto id = QUuid::createUuid();
	QTest::newRow("uuid") << QVariant{id}
						  << QJsonValue{id.toString(QUuid::WithoutBraces)}
						  << true
						  << QVariantHash{};
	QTest::newRow("url.valid") << QVariant{QUrl{QStringLiteral("https://example.com/test.xml?baum=42#tree")}}
							   << QJsonValue{QStringLiteral("https://example.com/test.xml?baum=42#tree")}
							   << true
							   << QVariantHash{};
	QTest::newRow("url.invalid") << QVariant{QUrl{}}
								 << QJsonValue{QString{}}
								 << true
								 << QVariantHash{};

	// type converters
	QTest::newRow("converter") << QVariant::fromValue<EnumContainer>({EnumContainer::Normal1, EnumContainer::FlagX})
							   << QJsonValue{QJsonObject{
										   {QStringLiteral("0"), QStringLiteral("Normal1")},
										   {QStringLiteral("1"), QStringLiteral("FlagX")}
									   }}
								 << true
								 << QVariantHash{};
}

void SerializerTest::resetProps()
{
	serializer->setAllowDefaultNull(false);
	serializer->setKeepObjectName(false);
	serializer->setEnumAsString(false);
	serializer->setValidateBase64(true);
	serializer->setUseBcp47Locale(true);
	serializer->setValidationFlags(QJsonSerializer::ValidationFlag::StrictBasicTypes);
	serializer->setPolymorphing(QJsonSerializer::Polymorphing::Enabled);
}

namespace  {

template<typename Type, typename Json>
static void test_type() {
	QJsonSerializer s;
	Type v;
	Json j;
	static_assert(std::is_same<Json, decltype(s.serialize(v))>::value, "Wrong value returned by expression");
	s.serializeTo(nullptr, v);
	s.serializeTo(v);
	s.deserialize(QJsonValue{}, qMetaTypeId<Type>());
	s.deserialize(QJsonValue{}, qMetaTypeId<Type>(), nullptr);
	s.deserialize<Type>(j);
	s.deserialize<Type>(j, nullptr);
	s.deserializeFrom(nullptr, qMetaTypeId<Type>());
	s.deserializeFrom(nullptr, qMetaTypeId<Type>(), nullptr);
	s.deserializeFrom(QByteArray{}, qMetaTypeId<Type>());
	s.deserializeFrom(QByteArray{}, qMetaTypeId<Type>(), nullptr);
	s.deserializeFrom<Type>(nullptr);
	s.deserializeFrom<Type>(nullptr, nullptr);
	s.deserializeFrom<Type>(QByteArray{});
	s.deserializeFrom<Type>(QByteArray{}, nullptr);
}

Q_DECL_UNUSED void static_compile_test()
{
	test_type<QVariant, QJsonValue>();
	test_type<bool, QJsonValue>();
	test_type<int, QJsonValue>();
	test_type<double, QJsonValue>();
	test_type<QString, QJsonValue>();
	test_type<TestObject*, QJsonObject>();
	test_type<QPointer<TestObject>, QJsonObject>();
	test_type<QSharedPointer<TestObject>, QJsonObject>();
	test_type<QList<TestObject*>, QJsonArray>();
	test_type<QMap<QString, TestObject*>, QJsonObject>();
	test_type<int, QJsonValue>();
	test_type<QString, QJsonValue>();
	test_type<QList<int>, QJsonArray>();
	test_type<QMap<QString, bool>, QJsonObject>();
	test_type<QPair<double, bool>, QJsonArray>();
	test_type<TestTuple, QJsonArray>();
	test_type<TestPair, QJsonArray>();
}

}

QTEST_MAIN(SerializerTest)

#include "tst_serializer.moc"
