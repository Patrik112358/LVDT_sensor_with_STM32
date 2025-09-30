#import "@preview/diatypst:0.5.0": *

// Project information variables
#let project_name = "LVDT Sensor with Nucleo"
#let project_description = "LVDT signal conditioning using only STM32G474 and external audio amplifier"
#let author_name = "Patrik Drozdík"
#let course_name = "SME"
#let presentation_date = "20.05.2024"

#show: slides.with(
  title: project_name,
  subtitle: project_description,
  date: presentation_date,
  authors: (author_name),
  header: [
    #grid(
      columns: (1fr, 1fr, auto),
      column-gutter: 0.5em,
      align: center,
      inset: 4pt,
      [
        B3B38#strong[SME]1
      ],
      [
        #text(weight: "bold")[#author_name]
      ],
      [
        #text(weight: "bold")[Driving LVDT with STM32G474 and LM4889]
      ],
    )
  ],
  // header-title: [
  //   #grid(
  //     columns: (1fr, 1fr),
  //     column-gutter: 0.5em,
  //     [
  //       B3B38#strong[SME]1
  //     ],
  //     [
  //       #text(weight: "bold")[#author_name]
  //     ],
  //   )
  // ],
  // header-subtitle: [],
  // Styling according to requirements
  ratio: 4 / 3, // Format 4:3 as specified
  layout: "medium",
  // title-color: blue.darken(60%),
  title-color: rgb("#0065bd"),
  toc: false, // No TOC for short presentation
  theme: "normal",
)

// Helper function for slide headers
#let slide_header(title) = {
  // align(left)[
  //   #text(weight: "bold")[#course_name, #author_name, #project_name]
  // ]
  // align(center)[
  //   #text(size: 1.5em, weight: "bold")[#title]
  // ]
}

// = Section

== Problem and Theory
#slide_header("Problem and Theory")

#grid(
  columns: (1fr, 1fr),
  gutter: 1em,
  [
    *Linear Variable Differential Transformer (LVDT)*

    - Non-contact position sensing device
    - Based on electromagnetic induction
    - One primary coil and two secondary coils
    - Ferrite core movement affects mutual inductance
    // - Output: Differential voltage proportional to displacement
    - *Output*: Voltages from secondary coils proportional to core position
    - $x_"core position" = k dot (U_"SEC1"-U_"SEC2") / (U_"SEC1"+U_"SEC2")$
  ],
  [
    #align(center + horizon)[
      #figure(
        image(
          "assets/LVDT_operating_principle.png",
          height: 100%,
        ), // Should show LVDT operating principle diagram
        caption: [LVDT Operating Principle],
      )
    ]
  ],
)

== Solution Design
#slide_header("Solution Design")

#grid(
  columns: (2fr, 3fr),
  gutter: 1em,
  [
    *System Components:*

    - *STM32G474* Nucleo board for signal generation and processing
    - *LM4889* External audio amplifier
      - Powers primary coil *10kHz* AC
    - ADC #strong[$120"kSa""/""s"$] sampling of secondary coil outputs
    - Digital signal processing for displacement calculation
      - *Goertzel algorithm*
  ],
  [
    #align(center + horizon)[
      #figure(
        image(
          "assets/solution_block_diagram.png",
          width: 100%,
        ), // Should show block diagram of the system
        caption: [System Block Diagram],
      )
    ]
  ],
)

== Implementation
#slide_header("Implementation")

#grid(
  columns: (1fr, 1fr),
  // gutter: 0.5em,
  [
    #grid(
      rows: 2,
      gutter: 0.5em,
      [
        *Hardware Implementation:*

        // - LVDT construction:
        //   - Custom wound coils on cylindrical former
        //   - Ferromagnetic core for displacement sensing
        - Nucleo board connections:
          - DAC output to audio amplifier
          - ADC inputs from secondary coils
        - Signal conditioning circuit for secondary outputs
          - Resistor divider
          - Protection diodes
      ],
      [
        #figure(
          image("assets/homemade_lvdt.jpg", width: 90%), // Should show photo of the LVDT sensor
          caption: [Homemade LVDT Sensor],
        )

      ]
    )
  ],
  [
    #grid(
      rows: (1fr, 1fr),
      align: center + horizon,
      gutter: 0.5em,
      [
        #figure(
          image(
            "assets/physical_circuit.jpg",
            height: 80%,
          ), // Should show photo of complete circuit setup
          caption: [Complete Circuit Setup],
        )
      ],
      [
        #figure(
          image(
            "assets/ADC.drawio.png",
            height: 80%,
          ), // Should show photo of complete system
          caption: [Sampling],
        )
      ],
    )
  ],
)

== Results and Performance
#slide_header("Results and Performance")

#grid(
  columns: (1fr, 1fr),
  gutter: 1em,
  [
    *Measurement Results:*

    - Linear range: ±27.5 mm
    // - Resolution: Y μm
    // - Accuracy: Z%
    // - Response time: P ms
    // - Temperature drift: Q μm/°C

    *Signal processing approach:*
    - Goertzel algorithm for FT at 10 kHz
    - Adjustable sample averaging
        #figure(
          image(
            "assets/serial_chart_recording.png",
            width: 90%,
          ), // Should show raw and processed signals
          caption: [Processed Signals],
        )
  ],
  [
    // #grid(
    //   rows: (1fr, 1fr),
    //   gutter: 0.5em,
    //   [
        #figure(
          image("assets/data_plotter.png", width: 90%), // Should show linearity performance graph
          caption: [Raw Signals],
        )
    //   ],
    //   [
    //   ]
    // )
  ],
)

// == Future Improvements
// #slide_header("Future Improvements")

// #grid(
//   columns: 1fr,
//   gutter: 1em,
//   [
//     *Potential Improvements:*

//     - Enhanced coil winding for better sensitivity
//     - Implementation of temperature compensation
//     - Miniaturization of the sensor design
//     - Integration with wireless data transmission
//     - Real-time visualization through companion app

//     *Applications:*
//     - Precision mechanical measurements
//     - Automated control systems
//     - Structural health monitoring
//   ]
// )
