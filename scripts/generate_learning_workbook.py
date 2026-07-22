from datetime import UTC, datetime
from pathlib import Path
from xml.sax.saxutils import escape
from zipfile import ZIP_DEFLATED, ZipFile


output_path = Path(__file__).resolve().parents[1] / "OrbitalForge_Daily_Schedule.xlsx"


daily_overview = [
    ["Day", "Date", "Status", "Main result", "Primary C++/CS focus", "Completion evidence"],
    ["Day 1", "2026-07-20", "Completed", "Project foundation and domain value types", "Compilation, linking, classes, constructors, operators, const, tests", "CMake build; Vec3 and Body tests"],
    ["Day 2", "2026-07-21", "Completed", "N-body gravity and physical diagnostics", "vector, references, ownership, O(N^2), numerical invariants", "Gravity and diagnostics implemented; 82/82 tests pass"],
    ["Day 3", "2026-07-22", "In Progress", "Complete simulation architecture", "RAII, lifetime, copy/move, strategy dispatch, numerical accuracy", "Runtime selection; comparison test; full suite passes"],
    ["Day 4", "2026-07-23", "Remaining", "Complete usable CLI application", "Parsing, errors, variant/optional, filesystem, streams, reproducibility", "Scenario-driven simulation; CSV output; CLI tests"],
    ["Day 5", "2026-07-24", "Remaining", "Applied DSA and benchmark laboratory", "Sorting, selection, hashing, heaps, graphs, complexity", "Measured benchmark plus working DSA reports"],
    ["Day 6", "2026-07-25", "Remaining", "CPU and memory optimization laboratory", "Profiling, allocation, cache, AoS/SoA, SIMD, threading", "Reference-validated speedups with measurements"],
    ["Day 7", "2026-07-26", "Remaining", "Scalable algorithm and release capstone", "Trees, Barnes-Hut, parallel traversal, sanitizers, release", "Validated scalable kernel and reproducible report"],
]


session_schedule = [
    ["Day", "Session", "Minutes", "Status", "Implementation task", "Concepts forced", "Verification / output", "Recall target"],
    ["Day 1", "Foundation", 240, "Completed", "Configure targets; implement Vec3 and Body", "Translation units; linking; value semantics; constructors; operators; const", "Build and focused tests pass", "Explain declaration, definition, linking, invariants, tolerance"],
    ["Day 2", "Physics core", 300, "Completed", "Implement SystemState, gravity, and diagnostics", "vector; references; ownership; pair traversal; O(N^2); invariants", "Gravity and diagnostics tests; 82/82 total", "Explain vector locality, invalidation, symmetry, conservation"],
    ["Day 3", "Map integrators", 45, "In Progress", "Trace Verlet step; identify copies, moves, allocations, force evaluations", "Mutation; aliasing; temporary lifetime; copy elision; numerical error", "Written architecture map", "State cost and accuracy order of each integrator"],
    ["Day 3", "Runtime selection", 90, "Remaining", "Parse integrator name into enum and dispatch through common signature", "enum class; string_view; optional; switch/function table; strategy", "Selection tests for valid and invalid names", "Explain enum dispatch versus virtual interface"],
    ["Day 3", "Lifetime audit", 60, "Remaining", "Audit state ownership and step temporaries", "RAII; storage duration; Rule of Zero/Five; moves; dangling references", "No owning raw pointers; ownership path documented", "Identify every owned and borrowed object"],
    ["Day 3", "Accuracy comparison", 90, "Remaining", "Compare all integrators on identical two-body orbit", "Relative error; convergence; stability; parameterized tests", "Energy drift and final-position table", "Explain RK4 cost and timestep tradeoff"],
    ["Day 3", "Recall and close", 30, "Remaining", "Explain architecture without code", "Technical communication; design judgment", "Five-minute explanation", "Ownership, dispatch, force evaluations, errors"],
    ["Day 4", "Scenario model", 45, "Remaining", "Create validated Scenario configuration type", "Aggregates; invariants; strong boundaries; defaults", "Scenario construction tests", "Separate immutable configuration from mutable state"],
    ["Day 4", "Parser", 120, "Remaining", "Implement line-based .orbit parser with line-level errors", "ifstream; string/string_view; from_chars; optional; variant; exceptions", "Malformed and valid parser tests", "Explain view lifetime and error strategy"],
    ["Day 4", "CLI routing", 90, "Remaining", "Implement simulate, compare, benchmark command routing", "argc/argv; exit codes; stdout/stderr; dependency boundaries", "Command and failure-path tests", "Explain core-versus-I/O separation"],
    ["Day 4", "Output pipeline", 90, "Remaining", "Write trajectory, diagnostics, and metadata files", "filesystem; streams; precision; buffering; RAII", "Deterministic CSV headers and rows", "Explain why I/O is excluded from kernel timing"],
    ["Day 4", "Integration tests", 75, "Remaining", "Run two-body and cluster scenarios through CLI", "Integration tests; temporary paths; deterministic random seeds", "Repeatable output and exit-code checks", "Differentiate unit and integration tests"],
    ["Day 5", "Benchmark harness", 90, "Remaining", "Add warmups, repetitions, samples, min/median/mean/stddev/p95", "steady_clock; distributions; DCE; optimization levels; bias", "Release benchmark table", "Explain trustworthy benchmark design"],
    ["Day 5", "Sorting and selection", 90, "Remaining", "Rank integrators; compute percentiles; select best k", "sort; stable_sort; partial_sort; nth_element; lower_bound; partition", "Ranking and percentile reports", "Give complexity and stability of each algorithm"],
    ["Day 5", "Hash table", 75, "Remaining", "Build name-to-index map and reject duplicate body names", "unordered_map; hashing; buckets; collisions; reserve; load factor", "Lookup and duplicate tests", "Compare unordered_map, map, sorted vector"],
    ["Day 5", "Heap", 60, "Remaining", "Maintain top-k closest approaches or worst drift runs", "priority_queue; heap invariant; O(log K) updates", "Top-k report and tests", "Explain min-heap/max-heap comparator direction"],
    ["Day 5", "Graph traversal", 90, "Remaining", "Build proximity graph and find connected components", "Adjacency list; BFS; DFS; queue; stack; O(V+E)", "Cluster membership report", "Explain graph construction versus traversal cost"],
    ["Day 5", "DSA foundations", 45, "Remaining", "Trace merge sort, binary search, and heap operations by hand", "Recursion; divide-and-conquer; arrays; lists; trees; DP vocabulary", "Complexity/data-layout comparison table", "Explain why list is poor for numerical kernels"],
    ["Day 5", "Scaling experiment", 60, "Remaining", "Benchmark doubling body counts and calculate ratios", "Empirical complexity; constant factors; crossover behavior", "N versus runtime and runtime-ratio table", "Relate observed scaling to O(N^2)"],
    ["Day 6", "Profile baseline", 60, "Remaining", "Profile release build and record hot-path percentage", "Sampling; call graphs; optimization flags; hardware metadata", "Saved baseline with environment", "Explain why profiling precedes optimization"],
    ["Day 6", "Allocation-free API", 75, "Remaining", "Reuse caller-owned acceleration and scratch buffers", "span; ownership; capacity; allocation cost; preconditions", "Before/after benchmark and tests", "Explain span lifetime and zero-cost access"],
    ["Day 6", "Memory hierarchy", 60, "Remaining", "Compare sequential and shuffled traversal", "Registers; cache lines; L1/L2/L3; TLB; prefetching; locality", "Traversal benchmark", "Explain performance difference despite equal Big-O"],
    ["Day 6", "AoS versus SoA", 120, "Remaining", "Implement dense numeric SoA gravity representation", "Layout; padding; alignment; working set; data-oriented design", "Correctness and multi-size benchmark", "Explain bandwidth wasted by nonnumeric Body fields"],
    ["Day 6", "Compiler and loops", 75, "Remaining", "Inspect vectorization and remove measured loop inefficiencies", "Inlining; SIMD; dependencies; aliasing; reassociation; Amdahl", "Optimization report or assembly observation", "Explain what blocked or enabled vectorization"],
    ["Day 6", "Parallel row kernel", 120, "Remaining", "Partition independent output rows across jthreads", "Races; mutexes; atomics; false sharing; reductions; load balance", "1/2/4-thread benchmark and correctness test", "Explain why row ownership avoids shared writes"],
    ["Day 6", "Optimization review", 45, "Remaining", "Keep or reject changes using evidence", "Speedup; accuracy; maintainability; constant versus complexity", "Decision table", "Explain every retained optimization"],
    ["Day 7", "Tree design", 90, "Remaining", "Design octree node, bounds, mass, center, children, body references", "Trees; recursive ownership; unique_ptr; flat-index alternatives", "Node invariants and ownership diagram", "Explain pointer tree versus flat tree"],
    ["Day 7", "Build and aggregate", 120, "Remaining", "Insert bodies and aggregate mass/center of mass", "Subdivision; recursion; postorder; degeneracy; O(N log N)", "Boundary and clustered-tree tests", "Explain termination and pathological input"],
    ["Day 7", "Barnes-Hut traversal", 120, "Remaining", "Approximate distant nodes using theta criterion", "Pruning; approximation; tree traversal; average O(N log N)", "Acceleration tests across theta values", "Explain theta accuracy/performance tradeoff"],
    ["Day 7", "Validation matrix", 90, "Remaining", "Compare direct and tree kernels across sizes/distributions", "Reference validation; RMS/max error; crossover point", "Accuracy, drift, runtime, memory table", "Explain acceptable error and crossover"],
    ["Day 7", "Parallel traversal", 75, "Remaining", "Parallelize read-only tree traversal by target ranges", "Immutable sharing; task granularity; synchronization boundaries", "Scaling benchmark", "Explain why traversal is easier than parallel tree build"],
    ["Day 7", "Safety tooling", 90, "Remaining", "Run warnings, ASan, UBSan, and TSan where available", "Undefined behavior; use-after-free; overflow; race detection", "Tool logs and pass/fail record", "Match sanitizer to defect class"],
    ["Day 7", "Release proof", 90, "Remaining", "Document build/run, examples, architecture, limitations, results", "Release discipline; reproducibility; technical communication", "Fresh build, tests, examples, performance report", "Deliver ten-minute project explanation"],
]


dsa_matrix = [
    ["Structure / algorithm", "Project application", "Complexity to know", "Why it belongs", "Status"],
    ["Dynamic array / vector", "Body and sample storage", "Index O(1); append amortized O(1); insert middle O(N)", "Contiguous locality for numeric work", "Completed"],
    ["Linked list", "Comparison study only", "Known-position insert O(1); search O(N)", "Learn pointer overhead and poor locality", "Remaining"],
    ["Sort", "Rank benchmark and accuracy results", "O(N log N)", "Complete ordered report", "Remaining"],
    ["Stable sort", "Preserve prior ordering for tied metrics", "O(N log N)", "Understand stability", "Remaining"],
    ["Partial sort", "Best k simulation configurations", "O(N log K)", "Avoid ordering irrelevant tail", "Remaining"],
    ["Nth element", "Median and p95 samples", "Average O(N)", "Selection without full sort", "Remaining"],
    ["Binary search / lower_bound", "Query sorted trajectory times", "O(log N)", "Fast ordered lookup", "Remaining"],
    ["Hash table", "Body name to vector index", "Average O(1), worst O(N)", "Fast identity lookup and duplicate detection", "Remaining"],
    ["Ordered map/tree", "Compare ordered reporting", "O(log N)", "Ordering and predictable complexity tradeoff", "Remaining"],
    ["Heap / priority queue", "Top-k closest approaches or drift", "Push/pop O(log K), top O(1)", "Streaming top-k result", "Remaining"],
    ["Queue", "Breadth-first proximity traversal", "Push/pop O(1)", "BFS frontier", "Remaining"],
    ["Stack", "Iterative depth-first traversal", "Push/pop O(1)", "DFS and recursion alternative", "Remaining"],
    ["Graph adjacency list", "Body proximity clusters", "Storage O(V+E); traversal O(V+E)", "Connected components on real snapshots", "Remaining"],
    ["Recursion", "Tree building and traversal", "Depends on tree height/work", "Natural hierarchical algorithm expression", "Remaining"],
    ["Octree", "Barnes-Hut force approximation", "Typical build/query leads near O(N log N)", "Changes gravity scalability", "Remaining"],
    ["Divide and conquer", "Spatial subdivision and merge-sort study", "Problem-dependent", "Core algorithm design pattern", "Remaining"],
    ["Dynamic programming", "Optional checkpoint selection under storage budget", "States times transitions", "Learn state, recurrence, base case", "Remaining"],
]


optimization_matrix = [
    ["Experiment", "Baseline", "Change", "Metric", "Correctness check", "Concept", "Status"],
    ["Scaling", "Direct gravity at N", "Double N", "Runtime ratio", "Same scenario family", "O(N^2) empirical growth", "Remaining"],
    ["Allocation", "New acceleration vector each call", "Reuse caller-owned buffer", "Median step time and allocations", "Acceleration tolerance", "Allocation cost and span", "Remaining"],
    ["Locality", "Sequential vector order", "Shuffled access order", "Traversal time", "Identical arithmetic result", "Spatial locality and prefetching", "Remaining"],
    ["Data layout", "Array of Body structures", "Structure of numeric arrays", "Kernel time and memory footprint", "Per-body acceleration tolerance", "Cache lines, AoS/SoA, SIMD", "Remaining"],
    ["Pair strategy", "Symmetric i-j updates", "Independent output rows", "Single-thread time", "Acceleration tolerance", "Dependencies and parallel readiness", "Remaining"],
    ["Thread scaling", "One row-kernel thread", "2/4/hardware threads", "Speedup and efficiency", "Reference tolerance; race tooling", "Work partitioning and Amdahl", "Remaining"],
    ["Algorithm", "Direct O(N^2) gravity", "Barnes-Hut tree", "Runtime, memory, crossover N", "Max/RMS acceleration error and drift", "Tree pruning and approximation", "Remaining"],
    ["Theta sweep", "Direct exact result", "Several theta values", "Speed versus error", "Reference matrix", "Accuracy/performance tradeoff", "Remaining"],
]


knowledge_checklist = [
    ["Category", "Concept", "Applied evidence required", "Status"],
    ["Language", "Lifetime and storage duration", "Trace ownership/borrowing through one simulation step", "Remaining"],
    ["Language", "Copy, move, Rule of Zero/Five", "Identify generated operations and measured copies", "Remaining"],
    ["Language", "Value categories", "Explain lvalue/rvalue behavior in project APIs", "Remaining"],
    ["Language", "RAII and exception safety", "Files/resources close correctly on failure", "Remaining"],
    ["Language", "Runtime versus compile-time polymorphism", "Defend integrator dispatch design", "Remaining"],
    ["Language", "Templates, deduction, concepts", "Understand generic test/statistics helpers", "Remaining"],
    ["Library", "Containers and invalidation", "Choose vector/map/unordered_map with reasons", "Remaining"],
    ["Library", "Iterators, ranges, algorithms", "Use algorithms in rankings and statistics", "Remaining"],
    ["Library", "optional and variant", "Parser success/error or optional lookup", "Remaining"],
    ["Library", "filesystem and streams", "Scenario input and deterministic output", "Remaining"],
    ["Library", "chrono and random", "Reliable timing and seeded scenarios", "Remaining"],
    ["Library", "threads, locks, atomics", "Race-free parallel kernel and explanation", "Remaining"],
    ["Systems", "Stack, heap, virtual memory", "Explain where major project objects/storage live", "Remaining"],
    ["Systems", "L1/L2/L3 and cache lines", "Locality experiment with measured result", "Remaining"],
    ["Systems", "Padding, alignment, AoS/SoA", "Layout sizes and force-kernel comparison", "Remaining"],
    ["Systems", "SIMD and vectorization", "Compiler report or assembly observation", "Remaining"],
    ["Systems", "Races and false sharing", "Thread design and sanitizer result", "Remaining"],
    ["Engineering", "Build targets and dependencies", "Explain CMake target graph", "Completed"],
    ["Engineering", "Unit/integration/regression/property tests", "Use appropriate layers in project", "Remaining"],
    ["Engineering", "Profiling and benchmark validity", "Reproducible environment-tagged results", "Remaining"],
    ["Engineering", "Sanitizers and undefined behavior", "Run available tools and classify findings", "Remaining"],
    ["Engineering", "Release and technical explanation", "Fresh build and ten-minute project walkthrough", "Remaining"],
]


evidence = [
    ["Date", "Area", "Evidence", "State"],
    ["2026-07-22", "Build", "cmake --build build succeeds", "Verified"],
    ["2026-07-22", "Tests", "ctest passes 82/82 tests", "Verified"],
    ["2026-07-22", "Core", "Vec3, Body, SystemState, gravity, diagnostics exist", "Verified"],
    ["2026-07-22", "Integrators", "Euler, semi-implicit Euler, Verlet, leapfrog, RK4 functions exist", "Verified"],
    ["2026-07-22", "Application", "Scenario parser, full CLI, and CSV pipeline are not complete", "Remaining"],
    ["2026-07-22", "Optimization", "Benchmark, cache study, parallel kernel, and Barnes-Hut are not complete", "Remaining"],
]


sheets = [
    ("Daily Overview", daily_overview, [13, 14, 15, 38, 54, 48]),
    ("Session Schedule", session_schedule, [11, 23, 10, 14, 55, 58, 46, 45]),
    ("DSA Matrix", dsa_matrix, [28, 48, 34, 48, 14]),
    ("Optimization Lab", optimization_matrix, [22, 35, 38, 31, 40, 40, 14]),
    ("Knowledge Checklist", knowledge_checklist, [18, 38, 62, 14]),
    ("Evidence", evidence, [14, 18, 72, 14]),
]


def column_name(index):
    name = ""
    while index:
        index, remainder = divmod(index - 1, 26)
        name = chr(65 + remainder) + name
    return name


def cell_xml(reference, value, style):
    if isinstance(value, (int, float)):
        return f'<c r="{reference}" s="{style}"><v>{value}</v></c>'
    safe_value = escape(str(value))
    return f'<c r="{reference}" s="{style}" t="inlineStr"><is><t xml:space="preserve">{safe_value}</t></is></c>'


def sheet_xml(rows, widths):
    columns = "".join(f'<col min="{index}" max="{index}" width="{width}" customWidth="1"/>' for index, width in enumerate(widths, 1))
    row_data = []
    for row_index, row in enumerate(rows, 1):
        cells = []
        for column_index, value in enumerate(row, 1):
            status_value = str(value)
            style = 1 if row_index == 1 else 2
            if status_value == "Completed" or status_value == "Verified":
                style = 3
            elif status_value == "In Progress":
                style = 4
            elif status_value == "Remaining":
                style = 5
            cells.append(cell_xml(f"{column_name(column_index)}{row_index}", value, style))
        height = 30 if row_index == 1 else 42
        row_data.append(f'<row r="{row_index}" ht="{height}" customHeight="1">{"".join(cells)}</row>')
    end_column = column_name(max(len(row) for row in rows))
    return f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<worksheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<sheetViews><sheetView workbookViewId="0"><pane ySplit="1" topLeftCell="A2" activePane="bottomLeft" state="frozen"/></sheetView></sheetViews>
<cols>{columns}</cols>
<sheetData>{"".join(row_data)}</sheetData>
<autoFilter ref="A1:{end_column}{len(rows)}"/>
<sheetFormatPr defaultRowHeight="18"/>
<pageMargins left="0.3" right="0.3" top="0.5" bottom="0.5" header="0.2" footer="0.2"/>
</worksheet>'''


content_types = ['<?xml version="1.0" encoding="UTF-8" standalone="yes"?>', '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">', '<Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>', '<Default Extension="xml" ContentType="application/xml"/>', '<Override PartName="/xl/workbook.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml"/>', '<Override PartName="/xl/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.styles+xml"/>', '<Override PartName="/docProps/core.xml" ContentType="application/vnd.openxmlformats-package.core-properties+xml"/>', '<Override PartName="/docProps/app.xml" ContentType="application/vnd.openxmlformats-officedocument.extended-properties+xml"/>']
for sheet_index in range(1, len(sheets) + 1):
    content_types.append(f'<Override PartName="/xl/worksheets/sheet{sheet_index}.xml" ContentType="application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"/>')
content_types.append('</Types>')


workbook_sheets = "".join(f'<sheet name="{escape(name)}" sheetId="{index}" r:id="rId{index}"/>' for index, (name, _, _) in enumerate(sheets, 1))
workbook_xml = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<workbook xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main" xmlns:r="http://schemas.openxmlformats.org/officeDocument/2006/relationships"><sheets>{workbook_sheets}</sheets></workbook>'''


relationship_entries = "".join(f'<Relationship Id="rId{index}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet" Target="worksheets/sheet{index}.xml"/>' for index in range(1, len(sheets) + 1))
relationship_entries += f'<Relationship Id="rId{len(sheets) + 1}" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles" Target="styles.xml"/>'
workbook_relationships = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">{relationship_entries}</Relationships>'''


styles_xml = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<styleSheet xmlns="http://schemas.openxmlformats.org/spreadsheetml/2006/main">
<fonts count="2"><font><sz val="10"/><name val="Aptos"/></font><font><b/><color rgb="FFFFFFFF"/><sz val="11"/><name val="Aptos Display"/></font></fonts>
<fills count="6"><fill><patternFill patternType="none"/></fill><fill><patternFill patternType="gray125"/></fill><fill><patternFill patternType="solid"><fgColor rgb="FF17324D"/><bgColor indexed="64"/></patternFill></fill><fill><patternFill patternType="solid"><fgColor rgb="FFD9EAD3"/><bgColor indexed="64"/></patternFill></fill><fill><patternFill patternType="solid"><fgColor rgb="FFFFE599"/><bgColor indexed="64"/></patternFill></fill><fill><patternFill patternType="solid"><fgColor rgb="FFF2F2F2"/><bgColor indexed="64"/></patternFill></fill></fills>
<borders count="2"><border><left/><right/><top/><bottom/><diagonal/></border><border><left style="thin"><color rgb="FFD0D7DE"/></left><right style="thin"><color rgb="FFD0D7DE"/></right><top style="thin"><color rgb="FFD0D7DE"/></top><bottom style="thin"><color rgb="FFD0D7DE"/></bottom><diagonal/></border></borders>
<cellStyleXfs count="1"><xf numFmtId="0" fontId="0" fillId="0" borderId="0"/></cellStyleXfs>
<cellXfs count="6"><xf numFmtId="0" fontId="0" fillId="0" borderId="0" xfId="0"/><xf numFmtId="0" fontId="1" fillId="2" borderId="1" xfId="0" applyAlignment="1"><alignment vertical="center" wrapText="1"/></xf><xf numFmtId="0" fontId="0" fillId="0" borderId="1" xfId="0" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf><xf numFmtId="0" fontId="0" fillId="3" borderId="1" xfId="0" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf><xf numFmtId="0" fontId="0" fillId="4" borderId="1" xfId="0" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf><xf numFmtId="0" fontId="0" fillId="5" borderId="1" xfId="0" applyAlignment="1"><alignment vertical="top" wrapText="1"/></xf></cellXfs>
<cellStyles count="1"><cellStyle name="Normal" xfId="0" builtinId="0"/></cellStyles>
</styleSheet>'''


root_relationships = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="xl/workbook.xml"/><Relationship Id="rId2" Type="http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties" Target="docProps/core.xml"/><Relationship Id="rId3" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/extended-properties" Target="docProps/app.xml"/></Relationships>'''


timestamp = datetime.now(UTC).replace(microsecond=0).isoformat().replace("+00:00", "Z")
core_xml = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<cp:coreProperties xmlns:cp="http://schemas.openxmlformats.org/package/2006/metadata/core-properties" xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:dcterms="http://purl.org/dc/terms/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"><dc:title>OrbitalForge Intensive C++ Learning Schedule</dc:title><dc:creator>OrbitalForge</dc:creator><dcterms:created xsi:type="dcterms:W3CDTF">{timestamp}</dcterms:created><dcterms:modified xsi:type="dcterms:W3CDTF">{timestamp}</dcterms:modified></cp:coreProperties>'''
app_xml = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Properties xmlns="http://schemas.openxmlformats.org/officeDocument/2006/extended-properties" xmlns:vt="http://schemas.openxmlformats.org/officeDocument/2006/docPropsVTypes"><Application>OrbitalForge</Application><Sheets>{len(sheets)}</Sheets></Properties>'''


with ZipFile(output_path, "w", ZIP_DEFLATED) as workbook:
    workbook.writestr("[Content_Types].xml", "".join(content_types))
    workbook.writestr("_rels/.rels", root_relationships)
    workbook.writestr("xl/workbook.xml", workbook_xml)
    workbook.writestr("xl/_rels/workbook.xml.rels", workbook_relationships)
    workbook.writestr("xl/styles.xml", styles_xml)
    workbook.writestr("docProps/core.xml", core_xml)
    workbook.writestr("docProps/app.xml", app_xml)
    for sheet_index, (_, rows, widths) in enumerate(sheets, 1):
        workbook.writestr(f"xl/worksheets/sheet{sheet_index}.xml", sheet_xml(rows, widths))


print(output_path)
