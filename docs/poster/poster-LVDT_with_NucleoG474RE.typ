#import "peace-of-posters-0.5.6/lib.typ" as pop
#import "@preview/xarrow:0.3.0": xarrow
#import "@local/ctu-graphical-assets:0.0.0" as ctu
#import "@preview/drafting:0.2.2"
#import "@preview/unify:0.7.1": add-unit, qty, unit
#add-unit("sample", "Sa", "upright(\"Sa\")", space: true)

#import "@preview/wrap-it:0.1.1": wrap-content

#set page("a1", margin: (top: 2cm, bottom: 2cm, left: 2cm, right: 2cm))
#pop.set-poster-layout(pop.layout-a1)
#pop.set-theme(pop.ctu-prague)
#set text(font: "Technika", size: pop.layout-a1.at("body-size"))
#let box-spacing = 1.2em
#set columns(gutter: box-spacing)
#set block(spacing: box-spacing)
#pop.update-poster-layout(spacing: box-spacing, heading-size: 30pt)
#set heading(numbering: "1.")
#show heading: set text(
  font: "Technika",
  size: 30pt,
  fill: ctu.colors-rgb.main.primary,
  weight: "bold",
)
// custom references for sections
#show ref: it => {
  if it.element != none and it.element.func() == heading {
    text(fill: ctu.colors-rgb.main.secondary_bw)[(#it)]
  } else {
    it
  }
}
// set equation numbering
#set math.equation(numbering: "(1)")
#show figure.where(kind: image): set figure(supplement: "Fig.")
// #show image: it => rect(it)
#let framed-image(..args) = rect(image(..args))
// #drafting.set-page-properties(
//   margin-left: 2cm,
//   margin-right: 2cm,
//   // margin-inside: 2cm,
//   // margin-outside: 2cm,
// )
#let todo(text, ..kwargs) = drafting.inline-note(text, par-break: false, ..kwargs)
#set table(stroke: (x, y) => {
  if y <= 1 {
    return (top: 1pt + black, bottom: 1pt + black)
  }
  if y > 1 {
    return (bottom: 1pt, top: 0pt)
  }
  return none
})

#let title-box(place_authors: true, place_subtitle: true) = {
  pop.title-box(
    [
      #v(-1.5cm)
      #set text(fill: white)
      // #image("psi_scd_banner_white.png", width: 35%)
      #{
        set image(height: 5cm)
        ctu.logos.electrical_engeneering_negativ
        v(-0.5em)
      }
      Driving and processing signals from LVDT sensor using NUCLEO-G474RE
      // Filled stick - _is it in yet?_
    ],
    subtitle: if place_subtitle [
      // #v(-0.5cm)
      #set text(
        fill: white,
        size: 30pt,
        // number-type: "old-style",
      )
      Semestral work for courses B3B38#underline[LPE]1 and B3B38#underline[SME]1
    ] else { none },
    authors: if place_authors [
      #v(1cm)
      #set text(fill: black)
      Patrik Drozdík#super("1")
    ] else { none },
    institutes: if place_authors [
      #set text(fill: black, weight: "regular")
      #super("1")Faculty of Electrical Engineering, Czech Technical University in Prague,
      Czech Republic
    ] else { none },
    background: box(
      rect(
        fill: gradient.linear(
          ctu.colors-rgb.main.primary,
          ctu.colors-rgb.main.secondary,
        ),
        height: 16cm,
        width: 100%,
      ),
      inset: -2cm,
      outset: 0pt,
    ),
    authors-size: 27pt,
    institutes-size: 19pt,
  )
}

#title-box()

#columns(
  2,
  [
    #pop.column-box(heading: [#heading(numbering: none)[Summary]])[
      - Drive and process signals from LVDT sensor using STM32 Nucleo board
        - Typically a dedicated LVDT signal conditioner is used, such as AD598, AD698, TI PGA970...
      - NUCLEO-G474RE is a low-cost development board with STM32G434RE microcontroller
        - 32-bit ARM Cortex-M4 core
        - up to 170MHz CPU clock
        - 5 *12-bit ADCs* 4 Msps, 3 *external DAC* channels
        - 2 8-channel *DMA controllers*, CORDIC trigonometric accelerator
      - Output measured data as CSV through UART
      - Show current values on OLED display
    ]

    #pop.column-box(heading: [= What is an LVDT sensor?])[
      #grid(
        columns: (auto, 10cm),
        inset: 0.3em,
        // align: center+horizon,
        [
          $
            "x"_"core" = k (A_"1" - A_"2") / (A_"1" + A_"2")
          $ <core_position>
          - where $A_"1"$ and $A_"2"$ are the amplitudes of secondary coils signals for the driving signal frequency
          - $k space.med["mm"]$ is the length-coefficient of the LVDT sensor
          - holds true within the linear range (ours $approx plus.minus 1.5 "cm"$)
        ],
        grid.cell(
          align: center + top,
          // inset: 0.3em,
          [#figure(
              framed-image("assets/LVDT_sensor_principle.svg", width: 100%),
              kind: image,
              caption: [LVDT sensor/* as a 1P2S transformer with a movable core */ ],
            ) <lvdt-principle-schematic>],
        ),
      )
    ]

    #pop.column-box(heading: [= Block schema])[
      #grid(
        // columns: (1fr, 2fr),
        // align: center+horizon,
        // [
        //   - MCU generates
        // ],
        grid.cell(align: center + top, inset: 0.3em, [#figure(
            framed-image("assets/plotted_schematic/NucleoLVDT.svg", width: 90%),
            kind: image,
            caption: [Block schema of the NUCLEO-G474RE LVDT sensor driver],
          ) <block-schema>]),
      )
    ]

    #pop.column-box(heading: [
      = Driving the primary coil - $10 "kHz" #text(font: "Technika")[sine wave]$
      <par-primary>])[
      #grid(
        columns: (1fr, 1fr),
        // align: center+horizon,
        [
          - Sine table programmed into FLASH
          - TIM6 generates interrupts at $10 unit("kHz") dot N_"samples" = 1 unit("MHz")$
          - DMA1 ch3 transfers data from FLASH to DAC1 ch1
          - DC voltage removed by blocking capacitor C3
          - LM4889 amplifier with $A = 1$ ensures low output impedance and power independence from MCU
        ],
        grid.cell(align: center + top, inset: 0.3em, [#figure(
            framed-image(
              "assets/plotted_schematic/NucleoLVDT-Breadboard-Primary_coil.svg",
              width: 100%,
            ),
            kind: image,
            caption: [Primary coil driving circuit],
          ) <primary-coil-driving-schematic>]),
      )

    ]

    // #pop.bibliography-box(
    //   "docs/content/showcase/2025-PSI/references.bib",
    //   style: "docs/content/showcase/2025-PSI/brief.csl",
    //   body-size: 0.55em,
    // )

    // #colbreak()

    #pop.column-box(heading: [= Measuring secondary coils - $120 "kSa/s"$
    <par-secondaries>])[
      #wrap-content(
        [#figure(
            framed-image(
              "assets/plotted_schematic/NucleoLVDT-Breadboard-Secondary_coil.svg",
              width: 100%,
            ),
            kind: image,
            caption: [Secondary coil measuring circuit],
          ) <secondary-coil-schematic>],
        [
          - connected trough blocking capacitors and voltage dividers
          - TIM7 generates interrupts at $120 unit("kHz")$
          - ADC1 & ADC2 in 12-bit Regular simultaneous mode sample both secondary coils at once
          - DMA1 ch1 transfers data from ADC1 & ADC2 to #qty(2, "kB") RAM buffer
          - after each halfbuffer transfer complete, interrupt handler transfers this halfbuffer into #qty(1, "kB") processing buffer
        ],
        align: right + top,
        columns: (1fr, 1fr),
      )
      // #grid(
      //   columns: (1fr, 1fr),
      //   // align: center+horizon,
      //   [
      //     - connected trough blocking capacitors and voltage dividers
      //     - TIM7 generates interrupts at $120 unit("kHz")$
      //     - ADC1 & ADC2 in 12-bit Regular simultaneous mode sample both secondary coils at once
      //     - DMA1 ch1 transfers data from ADC1 & ADC2 to memory
      //   ],
      //   grid.cell(
      //     align: center + top,
      //     inset: 0.3em,
      //     framed-image("assets/plotted_schematic/NucleoLVDT-Breadboard-Secondary_coil.svg", width: 100%),
      //   ),
      // )
    ]

    #pop.column-box(heading: [
      = Signal processing - $approx 110 "Hz" #text(font: "Technika", "update")$
    ])[
      - Goertzel algorithm used to calculate the amplitude of secondary coils for the driving frequency
        - CORDIC trigonometric accelerator used to calculate the magnitude from the real and imaginary parts
      - core position is calculated as per @core_position
    ]

    #colbreak()
    #place(top + left, scope: "parent", float: true, [#title-box(
        place_authors: false,
        place_subtitle: false,
      )
      #v(3cm)])

    #pop.column-box(heading: [= Data presentation <par-display>])[
      #wrap-content(
        [
          #figure(
            framed-image("assets/physical_display.jpg", width: 100%),
            kind: image,
            caption: [OLED display with current values],
          ) <oled-display>
          // #figure(
          //   framed-image("assets/serial_communication.png", width: 100%),
          //   kind: image,
          //   caption: [Serial communication with PC],
          // ) <serial-communication>,
        ],
        [
          - current values are shown on 128x64 OLED display
            - driving and sampling frequencies, calculated amplitudes, core position
            - core position visualized as horizontal slider
          - calculated amplitudes and core position are sent through USART1
        ],
        columns: (3fr, 3fr),
        align: right + top,
      )
    ]

    #pop.column-box(heading: [= Physical implementation])[
      - small 23-by-12 breadboard hosts the circuitry
        - amplifier for driving the primary @par-primary
        - voltage dividers and blocking capacitors for measuring secondaries @par-secondaries
        - 128x64 OLED display @par-display
      - connects to NUCLEO-G474RE through DuPont ribbons
        - 3w power, 1w primary drive, 2w secondary sense
        - 5w control signals for OLED display
      - connects to the LVDT sensor itself through 6 wires leading to each coils' ends
      #figure(
        image("assets/physical_circuit.jpg", width: 80%),
        kind: image,
        caption: [Breadboard featuring circuits from @primary-coil-driving-schematic, @secondary-coil-schematic and the OLED display],
      ) <breadboard-physical>
      #figure(
        image("assets/homemade_lvdt.jpg", width: 100%),
        kind: image,
        caption: [LVDT sensor made by doc. Petrucha],
      ) <lvdt-physical>

    ]

    #pop.column-box(heading: [= Step response])[
      - transfer characteristic of the used LVDT sensor was measured in @measurements-1mm-step-chart
        - $x$ axis is the core position as measured by vernier callipers, left $y$ axis is the amplitude of the secondary coils, right $y$ axis is the calculated core position divided by $k$
      - $k$ was calculated from the slope of the linear part of the transfer characteristic
        - $k = #{ calc.round(1 / 0.4312, digits: 3) } "mm"$ for our sensor
      - length of the linear range is $approx 2 dot 1.5 "cm" = 3 "cm"$
      #figure(
        framed-image(
          "assets/measurements-1mm_step-chart.svg",
          width: 90%,
        ),
        kind: image,
        caption: [Measured data - step response of the LVDT sensor],
      ) <measurements-1mm-step-chart>
    ]

    #pop.column-box(heading: [= Parameters overview])[
      #table(
        columns: (auto, auto),
        inset: 10pt,
        align: horizon,
        table.header([*Parameter*], [*Value*]),

        [Driving signal frequency], [#qty(10, "kHz")],
        [DAC Frequency], [#qty(1000, "MHz")],
        [DAC Resolution], [#qty(12, "bit")],
        [DAC Sample transfer method], [DMA],
        table.hline(start: 0, stroke: black),
        [ADC Sampling frequency], [#qty(120, "kSa/s", per: "fraction-short")],
        [ADC Resolution], [#qty(12, "bit")],
        [ADC Sample transfer method], [1 DMA for both ADCs],
        [ADC Buffer length], [#qty(2, "kSa")],
        [Calculation buffer length], [#qty(1, "kSa")],
        [Calculation buffers sampled / second],
        [$approx #qty(calc.round(120000 / 1024, digits: 2), "Hz")$],

        [Calculations performed / second], [$gt #qty(100, "Hz")$],
      )
    ]

    #pop.column-box(heading: [= Conclusion])[
      Both the physical circuitry and the firmware were successfully implemented. Thorugh the use of DMA and CORDIC, satisfying performance was achieved.

      The NUCLEO-G474RE board is more than capable to be used instead of dedicated LVDT signal conditioners, while allowing any other custom processing to take place, or even handle other sensors.
    ]

  ],
)
