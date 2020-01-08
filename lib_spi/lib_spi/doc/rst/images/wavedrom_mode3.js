{signal: [
  {name: 'CLK',  wave: '1..010101|01..', node: '...B'},
  {name: 'MOSI', wave: 'x..2.2.2.|2.x...', data: ['MSB',,, 'LSB'] },
  {name: 'MISO', wave: 'x..2.2.2.|2.x...', data: ['MSB',,, 'LSB'], node: '...................'},
  {name: 'CS',   wave: '10.......|..1..0', node: '.A..........C..D..'},
  {              node: '.a.b........c..d'}
],
  edge: [ 'A|a','B|b', 'C|c','D|d','a<->b t1',  'c<->d t2']
}