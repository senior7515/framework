<?hh
namespace Titon\Utility;

use Titon\Test\TestCase;
use Titon\Type\Contract\Arrayable;
use Titon\Type\Contract\Jsonable;
use Titon\Type\Contract\Xmlable;
use \Exception;
use \SimpleXMLElement;

class ConverterTest extends TestCase {

    protected $array;
    protected $object;
    protected $json;
    protected $serialized;
    protected $xml;
    protected $barbarian;

    protected function setUp() {
        parent::setUp();

        $data = ['key' => 'value', 'number' => 1337, 'boolean' => true, 'float' => 1.50, 'array' => [1, 2, 3]];

        $this->array = $data;

        // Object
        $object = new \stdClass();
        $object->key = 'value';
        $object->number = 1337;
        $object->boolean = true;
        $object->float = 1.50;
        $subObject = new \stdClass();
        $subObject->{'0'} = 1;
        $subObject->{'1'} = 2;
        $subObject->{'2'} = 3;
        $object->array = $subObject;

        $this->object = $object;

        // Json
        $this->json = json_encode($data);

        // Serialized
        $this->serialized = serialize($data);

        // XML
        $xml  = '<?xml version="1.0" encoding="utf-8"?>' . "\n";
        $xml .= '<root>';
        $xml .= '<key>value</key>';
        $xml .= '<number>1337</number>';
        $xml .= '<boolean>true</boolean>';
        $xml .= '<float>1.5</float>';
        $xml .= '<array>1</array><array>2</array><array>3</array>';
        $xml .= '</root>';

        $this->xml = $xml;
        $this->barbarian = file_get_contents(TEMP_DIR . '/utility/barbarian.xml');
    }

    public function testIs() {
        $this->assertEquals('array', Converter::is($this->array));
        $this->assertEquals('object', Converter::is($this->object));
        $this->assertEquals('json', Converter::is($this->json));
        $this->assertEquals('serialized', Converter::is($this->serialized));
        $this->assertEquals('xml', Converter::is($this->xml));
    }

    public function testIsMacro() {
        Converter::macro('isBoolean', function($value) {
            return is_bool($value);
        });

        $this->assertEquals('boolean', Converter::is(true));
    }

    public function testIsResource() {
        $f = fopen('php://input', 'r');

        $this->assertEquals('resource', Converter::is($f));

        fclose($f);
    }

    public function testIsArray() {
        $this->assertTrue(Converter::isArray($this->array));
        $this->assertFalse(Converter::isArray($this->object));
        $this->assertFalse(Converter::isArray($this->json));
        $this->assertFalse(Converter::isArray($this->serialized));
        $this->assertFalse(Converter::isArray($this->xml));
    }

    public function testIsObject() {
        $this->assertFalse(Converter::isObject($this->array));
        $this->assertTrue(Converter::isObject($this->object));
        $this->assertFalse(Converter::isObject($this->json));
        $this->assertFalse(Converter::isObject($this->serialized));
        $this->assertFalse(Converter::isObject($this->xml));
    }

    public function testIsJson() {
        $this->assertFalse((bool) Converter::isJson($this->array));
        $this->assertFalse((bool) Converter::isJson($this->object));
        $this->assertTrue((bool) Converter::isJson($this->json));
        $this->assertFalse((bool) Converter::isJson($this->serialized));
        $this->assertFalse((bool) Converter::isJson($this->xml));
    }

    public function testIsSerialized() {
        $this->assertFalse((bool) Converter::isSerialized($this->array));
        $this->assertFalse((bool) Converter::isSerialized($this->object));
        $this->assertFalse((bool) Converter::isSerialized($this->json));
        $this->assertTrue((bool) Converter::isSerialized($this->serialized));
        $this->assertFalse((bool) Converter::isSerialized($this->xml));
    }

    public function testIsXml() {
        $this->assertFalse((bool) Converter::isXml($this->array));
        $this->assertFalse((bool) Converter::isXml($this->object));
        $this->assertFalse((bool) Converter::isXml($this->json));
        $this->assertFalse((bool) Converter::isXml($this->serialized));
        $this->assertTrue((bool) Converter::isXml($this->xml));
    }

    public function testToArray() {
        $this->assertEquals($this->array, Converter::toArray($this->array));
        $this->assertEquals($this->array, Converter::toArray($this->object));
        $this->assertEquals($this->array, Converter::toArray($this->json));
        $this->assertEquals($this->array, Converter::toArray($this->serialized));
        $this->assertEquals($this->array, Converter::toArray($this->xml));
    }

    public function testToArrayContract() {
        $test = new TypeContractStub($this->array);
        $this->assertEquals($this->array, Converter::toArray($test));

        $test = new TypeContractStub(123);
        $this->assertEquals([123], Converter::toArray($test));
    }

    public function testToArrayRecursive() {
        $object1 = new \stdClass();
        $object1->key = 1;

        $object2 = new \stdClass();
        $object2->key = 3;

        $array = [
            $object1,
            ['key' => 2],
            $object2
        ];

        $this->assertEquals($array, Converter::toArray($array));
        $this->assertEquals([
            ['key' => 1],
            ['key' => 2],
            ['key' => 3]
        ], Converter::toArray($array, true));
    }

    public function testToJson() {
        $this->assertEquals($this->json, Converter::toJson($this->array));
        $this->assertEquals($this->json, Converter::toJson($this->object));
        $this->assertEquals($this->json, Converter::toJson($this->json));
        $this->assertEquals($this->json, Converter::toJson($this->serialized));
        $this->assertEquals($this->json, Converter::toJson($this->xml));
    }

    public function testToJsonContract() {
        $test = new TypeContractStub($this->array);
        $this->assertEquals($this->json, Converter::toJson($test));

        $test = new TypeContractStub(['a' => 1]);
        $this->assertEquals('{"a":1}', Converter::toJson($test));
    }

    public function testToMap() {
        $this->assertEquals(Map {}, Converter::toMap(Vector {}));
        $this->assertEquals(Map {0 => 'foo'}, Converter::toMap(Vector {'foo'}));
        $this->assertEquals(Map {}, Converter::toMap([]));
        $this->assertEquals(Map {0 => 'foo'}, Converter::toMap(['foo']));
        $this->assertEquals(Map {0 => 'foo'}, Converter::toMap('foo'));
        $this->assertEquals(Map {0 => 123}, Converter::toMap(123));
        $this->assertEquals(Map {}, Converter::toMap(Map {}));
        $this->assertEquals(Map {'foo' => 'bar'}, Converter::toMap(Map {'foo' => 'bar'}));
    }

    public function testToSerialize() {
        $this->assertEquals($this->serialized, Converter::toSerialize($this->array));
        $this->assertEquals($this->serialized, Converter::toSerialize($this->object));
        $this->assertEquals($this->serialized, Converter::toSerialize($this->json));
        $this->assertEquals($this->serialized, Converter::toSerialize($this->serialized));
        $this->assertEquals($this->serialized, Converter::toSerialize($this->xml));
    }

    public function testToSerializeContract() {
        $test = new TypeContractStub($this->array);
        $this->assertEquals('C:30:"Titon\Utility\TypeContractStub":126:{' . $this->serialized . '}', Converter::toSerialize($test));
    }

    public function testToXml() {
        $this->assertEquals($this->xml, Converter::toXml($this->array));
        $this->assertEquals($this->xml, Converter::toXml($this->object));
        $this->assertEquals($this->xml, Converter::toXml($this->json));
        $this->assertEquals($this->xml, Converter::toXml($this->serialized));
        $this->assertEquals($this->xml, Converter::toXml($this->xml));
    }

    public function testToXmlContract() {
        $test = new TypeContractStub($this->array);
        $this->assertEquals($this->xml, Converter::toXml($test));

        $test = new TypeContractStub(['a' => 1]);
        $this->assertEquals('<?xml version="1.0" encoding="utf-8"?>' . "\n" . '<root><a>1</a></root>', Converter::toXml($test));
    }

    /**
     * @expectedException \PHPUnit_Framework_Error_Warning
     */
    public function testToXmlCantStartWithNumbers() {
        $items = [
            $this->createXmlItem(1),
            $this->createXmlItem(2),
            $this->createXmlItem(3)
        ];

        // Without named indices
        $expected  = '<?xml version="1.0" encoding="utf-8"?>' . "\n" . '<root>';
        $expected .= '<0><id>1</id><title>Item #1</title></0>';
        $expected .= '<1><id>2</id><title>Item #2</title></1>';
        $expected .= '<2><id>3</id><title>Item #3</title></2>';
        $expected .= '</root>';

        // XML nodes cant start with numbers
        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($items));
    }

    public function testToXmlWithNumericIndexes() {
        $numItems = ['item' => [
            1 => $this->createXmlItem(1)
        ]];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><root>';
        $expected .= '<item><id>1</id><title>Item #1</title></item>';
        $expected .= '</root>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($numItems));
    }

    public function testToXmlNestedIndexes() {
        $items = ['item' => [
            $this->createXmlItem(1),
            $this->createXmlItem(2),
            $this->createXmlItem(3)
        ]];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><root>';
        $expected .= '<item><id>1</id><title>Item #1</title></item>';
        $expected .= '<item><id>2</id><title>Item #2</title></item>';
        $expected .= '<item><id>3</id><title>Item #3</title></item>';
        $expected .= '</root>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($items));
    }

    public function testToXmlNestedDeep() {
        $items = ['item' => [
            $this->createXmlItem(1, true),
            $this->createXmlItem(2, true),
            $this->createXmlItem(3, true)
        ]];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><root>';
        $expected .= '<item>
            <id>1</id><title>Item #1</title>
            <foo><id>1</id><title>Item #1</title></foo>
            <foo><id>2</id><title>Item #2</title></foo>
            <foo><id>3</id><title>Item #3</title></foo>
        </item>';
        $expected .= '<item>
            <id>2</id><title>Item #2</title>
            <foo><id>1</id><title>Item #1</title></foo>
            <foo><id>2</id><title>Item #2</title></foo>
            <foo><id>3</id><title>Item #3</title></foo>
        </item>';
        $expected .= '<item>
            <id>3</id><title>Item #3</title>
            <foo><id>1</id><title>Item #1</title></foo>
            <foo><id>2</id><title>Item #2</title></foo>
            <foo><id>3</id><title>Item #3</title></foo>
        </item>';
        $expected .= '</root>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($items));
    }

    public function testToXmlNestedDeepAgain() {
        $items = ['item' => [
            $this->createXmlItem(1, 'a'),
            $this->createXmlItem(2, 'b'),
            $this->createXmlItem(3, 'c')
        ]];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><items>';
        $expected .= '<item>
            <id>1</id><title>Item #1</title>
            <foo>
                <a><id>1</id><title>Item #1</title></a>
                <a><id>2</id><title>Item #2</title></a>
                <a><id>3</id><title>Item #3</title></a>
            </foo>
        </item>';
        $expected .= '<item>
            <id>2</id><title>Item #2</title>
            <foo>
                <b><id>1</id><title>Item #1</title></b>
                <b><id>2</id><title>Item #2</title></b>
                <b><id>3</id><title>Item #3</title></b>
            </foo>
        </item>';
        $expected .= '<item>
            <id>3</id><title>Item #3</title>
            <foo>
                <c><id>1</id><title>Item #1</title></c>
                <c><id>2</id><title>Item #2</title></c>
                <c><id>3</id><title>Item #3</title></c>
            </foo>
        </item>';
        $expected .= '</items>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($items, 'items'));
    }

    public function testToXmlArrayOfTypes() {
        $items = ['item' => [
            (object) $this->createXmlItem(1),
            $this->createXmlItem(2),
            (object) $this->createXmlItem(3)
        ]];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><root>';
        $expected .= '<item><id>1</id><title>Item #1</title></item>';
        $expected .= '<item><id>2</id><title>Item #2</title></item>';
        $expected .= '<item><id>3</id><title>Item #3</title></item>';
        $expected .= '</root>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($items));
    }

    public function testToVector() {
        $this->assertEquals(Vector {}, Converter::toVector(Vector {}));
        $this->assertEquals(Vector {'foo'}, Converter::toVector(Vector {'foo'}));
        $this->assertEquals(Vector {}, Converter::toVector([]));
        $this->assertEquals(Vector {'foo'}, Converter::toVector(['foo']));
        $this->assertEquals(Vector {'foo'}, Converter::toVector('foo'));
        $this->assertEquals(Vector {123}, Converter::toVector(123));
        $this->assertEquals(Vector {}, Converter::toVector(Map {}));
        $this->assertEquals(Vector {'bar'}, Converter::toVector(Map {'foo' => 'bar'}));
    }

    public function testXmlTypeCasting() {
        $data = [
            'true' => true,
            'false' => false,
            'null' => null,
            'zero' => 0,
            'empty' => '',
            'float' => 1.50,
            'int' => 666
        ];

        $expected  = '<?xml version="1.0" encoding="utf-8"?><root>';
        $expected .= '<true>true</true>';
        $expected .= '<false>false</false>';
        $expected .= '<null></null>';
        $expected .= '<zero>0</zero>';
        $expected .= '<empty></empty>';
        $expected .= '<float>1.5</float>';
        $expected .= '<int>666</int>';
        $expected .= '</root>';

        $this->assertXmlStringEqualsXmlString($expected, Converter::toXml($data));

        $this->assertEquals($data, Converter::xmlToArray(new SimpleXMLElement($expected)));
    }

    public function testXmlToArrayNone() {
        $expected = [
            'name' => 'Barbarian',
            'life' => 50,
            'mana' => 100,
            'stamina' => 15,
            'vitality' => 20,
            'dexterity' => '',
            'agility' => '',
            'armors' => [
                'armor' => ['Helmet', 'Shoulder Plates', 'Breast Plate', 'Greaves', 'Gloves', 'Shield']
            ],
            'weapons' => [
                'sword' => ['Broadsword', 'Longsword'],
                'axe' => ['Heavy Axe', 'Double-edged Axe'],
                'polearm' => 'Polearm',
                'mace' => 'Mace'
            ],
            'items' => [
                'potions' => [
                    'potion' => ['Health Potion', 'Mana Potion']
                ],
                'keys' => [
                    'chestKey' => 'Chest Key',
                    'bossKey' => 'Boss Key'
                ],
                'food' => ['Fruit', 'Bread', 'Vegetables'],
                'scrap' => 'Scrap'
            ]
        ];

        $this->assertEquals($expected, Converter::xmlToArray(simplexml_load_string($this->barbarian), Converter::XML_NONE));

        $xml = <<<XML
<unit>
    <name>Barbarian</name>
    <life>50</life>
    <mana>100</mana>
    <stamina>15</stamina>
    <vitality>20</vitality>
    <dexterity/>
    <agility/>
    <armors>
        <armor>Helmet</armor>
        <armor>Shoulder Plates</armor>
        <armor>Breast Plate</armor>
        <armor>Greaves</armor>
        <armor>Gloves</armor>
        <armor>Shield</armor>
    </armors>
    <weapons>
        <sword>Broadsword</sword>
        <sword>Longsword</sword>
        <axe>Heavy Axe</axe>
        <axe>Double-edged Axe</axe>
        <polearm>Polearm</polearm>
        <mace>Mace</mace>
    </weapons>
    <items>
        <potions>
            <potion>Health Potion</potion>
            <potion>Mana Potion</potion>
        </potions>
        <keys>
            <chestKey>Chest Key</chestKey>
            <bossKey>Boss Key</bossKey>
        </keys>
        <food>Fruit</food>
        <food>Bread</food>
        <food>Vegetables</food>
        <scrap>Scrap</scrap>
    </items>
</unit>
XML;

        $xmlObject = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><unit></unit>');
        $this->assertEquals($this->prepareExpectedXml($xml), Converter::buildXml($xmlObject, $expected)->asXML());
    }

    public function testXmlToArrayMerge() {
        $expected = [
            'name' => 'Barbarian',
            'life' => ['value' => 50, 'max' => 150],
            'mana' => ['value' => 100, 'max' => 250],
            'stamina' => 15,
            'vitality' => 20,
            'dexterity' => ['value' => '', 'evade' => '5%', 'block' => '10%'],
            'agility' => ['value' => '', 'turnRate' => '1.25', 'acceleration' => 5],
            'armors' => [
                'armor' => [
                    ['value' => 'Helmet', 'defense' => 15],
                    ['value' => 'Shoulder Plates', 'defense' => 25],
                    ['value' => 'Breast Plate', 'defense' => 50],
                    ['value' => 'Greaves', 'defense' => 10],
                    ['value' => 'Gloves', 'defense' => 10],
                    ['value' => 'Shield', 'defense' => 25],
                ],
                'items' => 6
            ],
            'weapons' => [
                'sword' => [
                    ['value' => 'Broadsword', 'damage' => 25],
                    ['value' => 'Longsword', 'damage' => 30]
                ],
                'axe' => [
                    ['value' => 'Heavy Axe', 'damage' => 20],
                    ['value' => 'Double-edged Axe', 'damage' => 25],
                ],
                'polearm' => ['value' => 'Polearm', 'damage' => 50, 'range' => 3, 'speed' => 'slow'],
                'mace' => ['value' => 'Mace', 'damage' => 15, 'speed' => 'fast'],
                'items' => 6
            ],
            'items' => [
                'potions' => [
                    'potion' => ['Health Potion', 'Mana Potion']
                ],
                'keys' => [
                    'chestKey' => 'Chest Key',
                    'bossKey' => 'Boss Key'
                ],
                'food' => ['Fruit', 'Bread', 'Vegetables'],
                'scrap' => ['value' => 'Scrap', 'count' => 25]
            ]
        ];

        $this->assertEquals($expected, Converter::xmlToArray(simplexml_load_string($this->barbarian), Converter::XML_MERGE));

        $xml = <<<XML
<unit>
    <name>Barbarian</name>
    <life max="150">50</life>
    <mana max="250">100</mana>
    <stamina>15</stamina>
    <vitality>20</vitality>
    <dexterity evade="5%" block="10%"/>
    <agility turnRate="1.25" acceleration="5"/>
    <armors>
        <armor defense="15">Helmet</armor>
        <armor defense="25">Shoulder Plates</armor>
        <armor defense="50">Breast Plate</armor>
        <armor defense="10">Greaves</armor>
        <armor defense="10">Gloves</armor>
        <armor defense="25">Shield</armor>
        <items>6</items>
    </armors>
    <weapons>
        <sword damage="25">Broadsword</sword>
        <sword damage="30">Longsword</sword>
        <axe damage="20">Heavy Axe</axe>
        <axe damage="25">Double-edged Axe</axe>
        <polearm damage="50" range="3" speed="slow">Polearm</polearm>
        <mace damage="15" speed="fast">Mace</mace>
        <items>6</items>
    </weapons>
    <items>
        <potions>
            <potion>Health Potion</potion>
            <potion>Mana Potion</potion>
        </potions>
        <keys>
            <chestKey>Chest Key</chestKey>
            <bossKey>Boss Key</bossKey>
        </keys>
        <food>Fruit</food>
        <food>Bread</food>
        <food>Vegetables</food>
        <scrap count="25">Scrap</scrap>
    </items>
</unit>
XML;

        $xmlObject = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><unit></unit>');
        $this->assertEquals($this->prepareExpectedXml($xml), Converter::buildXml($xmlObject, $expected)->asXML());
    }

    public function testXmlToArrayGroup() {
        $expected = [
            'name' => 'Barbarian',
            'life' => [
                'value' => 50,
                'attributes' => ['max' => 150]
            ],
            'mana' => [
                'value' => 100,
                'attributes' => ['max' => 250]
            ],
            'stamina' => 15,
            'vitality' => 20,
            'dexterity' => [
                'value' => '',
                'attributes' => ['evade' => '5%', 'block' => '10%']
            ],
            'agility' => [
                'value' => '',
                'attributes' => ['turnRate' => '1.25', 'acceleration' => 5]
            ],
            'armors' => [
                'value' => [
                    'armor' => [
                        ['value' => 'Helmet', 'attributes' => ['defense' => 15]],
                        ['value' => 'Shoulder Plates', 'attributes' => ['defense' => 25]],
                        ['value' => 'Breast Plate', 'attributes' => ['defense' => 50]],
                        ['value' => 'Greaves', 'attributes' => ['defense' => 10]],
                        ['value' => 'Gloves', 'attributes' => ['defense' => 10]],
                        ['value' => 'Shield', 'attributes' => ['defense' => 25]],
                    ],
                ],
                'attributes' => ['items' => 6]
            ],
            'weapons' => [
                'value' => [
                    'sword' => [
                        ['value' => 'Broadsword', 'attributes' => ['damage' => 25]],
                        ['value' => 'Longsword', 'attributes' => ['damage' => 30]]
                    ],
                    'axe' => [
                        ['value' => 'Heavy Axe', 'attributes' => ['damage' => 20]],
                        ['value' => 'Double-edged Axe', 'attributes' => ['damage' => 25]],
                    ],
                    'polearm' => [
                        'value' => 'Polearm',
                        'attributes' => ['damage' => 50, 'range' => 3, 'speed' => 'slow']
                    ],
                    'mace' => [
                        'value' => 'Mace',
                        'attributes' => ['damage' => 15, 'speed' => 'fast']
                    ],
                ],
                'attributes' => ['items' => 6]
            ],
            'items' => [
                'potions' => [
                    'potion' => ['Health Potion', 'Mana Potion']
                ],
                'keys' => [
                    'chestKey' => 'Chest Key',
                    'bossKey' => 'Boss Key'
                ],
                'food' => ['Fruit', 'Bread', 'Vegetables'],
                'scrap' => [
                    'value' => 'Scrap',
                    'attributes' => ['count' => 25]
                ]
            ]
        ];

        $this->assertEquals($expected, Converter::xmlToArray(simplexml_load_string($this->barbarian), Converter::XML_GROUP));

        // Unset the value so that it auto-nulls
        unset($expected['agility']['value']);

        $xml = <<<XML
<unit>
    <name>Barbarian</name>
    <life max="150">50</life>
    <mana max="250">100</mana>
    <stamina>15</stamina>
    <vitality>20</vitality>
    <dexterity evade="5%" block="10%"/>
    <agility turnRate="1.25" acceleration="5"/>
    <armors items="6">
        <armor defense="15">Helmet</armor>
        <armor defense="25">Shoulder Plates</armor>
        <armor defense="50">Breast Plate</armor>
        <armor defense="10">Greaves</armor>
        <armor defense="10">Gloves</armor>
        <armor defense="25">Shield</armor>
    </armors>
    <weapons items="6">
        <sword damage="25">Broadsword</sword>
        <sword damage="30">Longsword</sword>
        <axe damage="20">Heavy Axe</axe>
        <axe damage="25">Double-edged Axe</axe>
        <polearm damage="50" range="3" speed="slow">Polearm</polearm>
        <mace damage="15" speed="fast">Mace</mace>
    </weapons>
    <items>
        <potions>
            <potion>Health Potion</potion>
            <potion>Mana Potion</potion>
        </potions>
        <keys>
            <chestKey>Chest Key</chestKey>
            <bossKey>Boss Key</bossKey>
        </keys>
        <food>Fruit</food>
        <food>Bread</food>
        <food>Vegetables</food>
        <scrap count="25">Scrap</scrap>
    </items>
</unit>
XML;

        $xmlObject = simplexml_load_string('<?xml version="1.0" encoding="utf-8"?><unit></unit>');
        $this->assertEquals($this->prepareExpectedXml($xml), Converter::buildXml($xmlObject, $expected)->asXML());
    }

    public function testXmlToArrayAttribs() {
        $expected = [
            'name' => 'Barbarian',
            'life' => ['max' => 150],
            'mana' => ['max' => 250],
            'stamina' => 15,
            'vitality' => 20,
            'dexterity' => ['evade' => '5%', 'block' => '10%'],
            'agility' => ['turnRate' => '1.25', 'acceleration' => 5],
            'armors' => [
                'items' => 6
            ],
            'weapons' => [
                'items' => 6
            ],
            'items' => [
                'potions' => [
                    'potion' => ['Health Potion', 'Mana Potion']
                ],
                'keys' => [
                    'chestKey' => 'Chest Key',
                    'bossKey' => 'Boss Key'
                ],
                'food' => ['Fruit', 'Bread', 'Vegetables'],
                'scrap' => ['count' => 25]
            ]
        ];

        $this->assertEquals($expected, Converter::xmlToArray(simplexml_load_string($this->barbarian), Converter::XML_ATTRIBS));
    }

    protected function createXmlItem($id, $complex = false) {
        $item = ['id' => $id, 'title' => 'Item #' . $id];

        if ($complex) {
            $item['foo'] = [
                $this->createXmlItem(1),
                $this->createXmlItem(2),
                $this->createXmlItem(3)
            ];

            if ($complex !== true) {
                $item['foo'] = [$complex => $item['foo']];
            }
        }

        return $item;
    }

    protected function prepareExpectedXml($string) {
        return '<?xml version="1.0" encoding="utf-8"?>' . "\n" . str_replace('    ', '', $this->clean($string)) . "\n";
    }

}

class TypeContractStub implements Arrayable, Jsonable, Xmlable, \Serializable {

    protected $data = [];

    public function __construct($data) {
        $this->data = $data;
    }

    public function toArray(): array {
        return (array) $this->data;
    }

    public function toJson(int $options = 0): string {
        return json_encode($this->data, $options);
    }

    public function toXml(string $root = 'root'): string {
        return Converter::toXml($this->toArray(), $root);
    }

    public function serialize() {
        return serialize($this->data);
    }

    public function unserialize($data) {
        $this->data = unserialize($data);
    }

}