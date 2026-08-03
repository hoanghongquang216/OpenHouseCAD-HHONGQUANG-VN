// Spiral 2 demo (DOC-003, Milestone 2.4): proves Layer is a real,
// functioning consumer of the renderer, not stored-but-unused data.
//
//   Layer "Walls"  -- black, solid   -> must appear
//   Layer "Center" -- gray, dashed   -> must appear, with stroke-dasharray
//   Layer "Hidden" -- visible=false  -> must NOT appear in the output at all
#include <openhouse/render/RenderDocument.hpp>
#include <cstdio>

int main(int argc, char** argv) {
    const char* outPath = (argc > 1) ? argv[1] : "layers_demo.svg";
    using namespace openhouse::document;
    using namespace openhouse::geometry;
    using namespace openhouse::render;

    Document doc;

    Layer& walls = doc.CreateLayer("Walls");
    walls.SetColor("black");
    walls.SetLineWeight(2.0);

    Layer& center = doc.CreateLayer("Center");
    center.SetColor("gray");
    center.SetLineType(LineType::Dashed);
    center.SetLineWeight(1.0);

    Layer& hidden = doc.CreateLayer("Hidden");
    hidden.SetColor("red");
    hidden.SetVisible(false);

    // Walls: a simple rectangle outline.
    doc.Add(Line2d{Point2d{20.0, 20.0}, Point2d{180.0, 20.0}}, "Walls");
    doc.Add(Line2d{Point2d{180.0, 20.0}, Point2d{180.0, 180.0}}, "Walls");
    doc.Add(Line2d{Point2d{180.0, 180.0}, Point2d{20.0, 180.0}}, "Walls");
    doc.Add(Line2d{Point2d{20.0, 180.0}, Point2d{20.0, 20.0}}, "Walls");

    // Center: a centerline cross.
    doc.Add(Line2d{Point2d{100.0, 20.0}, Point2d{100.0, 180.0}}, "Center");
    doc.Add(Line2d{Point2d{20.0, 100.0}, Point2d{180.0, 100.0}}, "Center");

    // Hidden: should never show up in the output SVG at all.
    doc.Add(Circle2d{Point2d{100.0, 100.0}, 30.0}, "Hidden");

    SvgDocument svg(200.0, 200.0);
    RenderToSvg(doc, svg);

    if (!svg.WriteToFile(outPath)) {
        std::fprintf(stderr, "Failed to write %s\n", outPath);
        return 1;
    }

    std::printf("Document: %zu entities across %zu layers.\n", doc.Count(), doc.Layers().size());
    std::printf("Wrote %s\n", outPath);
    return 0;
}
