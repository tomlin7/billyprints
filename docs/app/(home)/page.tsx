import Link from 'next/link';
import {
  CircuitBoard,
  Boxes,
  Code2,
  Database,
  Zap,
  Layers,
  Cpu,
  ArrowRight,
  BookOpen,
  Sparkles,
} from 'lucide-react';

export default function HomePage() {
  return (
    <main className="flex-1">
      {/* Hero Section */}
      <section className="relative overflow-hidden border-b border-fd-border">
        {/* Background Effects */}
        <div className="absolute inset-0 bg-gradient-to-br from-fd-primary/5 via-transparent to-fd-secondary/10" />
        <div className="absolute inset-0 bg-[radial-gradient(ellipse_at_top,rgba(var(--primary-rgb),0.1),transparent_50%)]" />
        <div className="absolute inset-0 bg-[url('data:image/svg+xml;base64,PHN2ZyB3aWR0aD0iNjAiIGhlaWdodD0iNjAiIHZpZXdCb3g9IjAgMCA2MCA2MCIgeG1sbnM9Imh0dHA6Ly93d3cudzMub3JnLzIwMDAvc3ZnIj48ZyBmaWxsPSJub25lIiBmaWxsLXJ1bGU9ImV2ZW5vZGQiPjxnIGZpbGw9IiMyMjIiIGZpbGwtb3BhY2l0eT0iMC4wMyI+PGNpcmNsZSBjeD0iMSIgY3k9IjEiIHI9IjEiLz48L2c+PC9nPjwvc3ZnPg==')] opacity-50" />

        <div className="relative mx-auto max-w-6xl px-6 py-24 sm:py-32 lg:py-40">
          <div className="text-center">
            {/* Badge */}
            <div className="mb-8 inline-flex items-center gap-2 rounded-full border border-fd-border bg-fd-card/80 px-4 py-1.5 text-sm backdrop-blur-sm">
              <Sparkles className="h-4 w-4 text-fd-primary" />
              <span className="text-fd-muted-foreground">Open Source Logic Simulator</span>
            </div>

            {/* Logo */}
            <div className="mx-auto mb-8 flex h-20 w-20 items-center justify-center rounded-2xl bg-gradient-to-br from-fd-primary to-fd-primary/70 shadow-xl shadow-fd-primary/20 ring-1 ring-white/10">
              <CircuitBoard className="h-10 w-10 text-white" />
            </div>

            {/* Title */}
            <h1 className="bg-gradient-to-b from-fd-foreground to-fd-foreground/70 bg-clip-text text-5xl font-bold tracking-tight text-transparent sm:text-6xl lg:text-7xl">
              Billyprints
            </h1>

            {/* Subtitle */}
            <p className="mx-auto mt-6 max-w-2xl text-lg text-fd-muted-foreground sm:text-xl leading-relaxed">
              A powerful node-based logic gate editor and simulator.
              <br className="hidden sm:block" />
              Build digital circuits{' '}
              <span className="text-fd-foreground font-medium">visually</span> or through{' '}
              <span className="text-fd-foreground font-medium">code</span>.
            </p>

            {/* CTA Buttons */}
            <div className="mt-10 flex flex-col items-center justify-center gap-4 sm:flex-row">
              <Link
                href="/docs"
                className="group inline-flex items-center justify-center gap-2 rounded-xl bg-fd-primary px-6 py-3.5 text-sm font-semibold text-fd-primary-foreground shadow-lg shadow-fd-primary/25 transition-all hover:bg-fd-primary/90 hover:shadow-xl hover:shadow-fd-primary/30 hover:-translate-y-0.5"
              >
                <BookOpen className="h-4 w-4" />
                Get Started
                <ArrowRight className="h-4 w-4 transition-transform group-hover:translate-x-0.5" />
              </Link>
              <Link
                href="/docs/quickstart"
                className="inline-flex items-center justify-center gap-2 rounded-xl border border-fd-border bg-fd-card/50 px-6 py-3.5 text-sm font-semibold text-fd-foreground backdrop-blur-sm transition-all hover:bg-fd-accent hover:border-fd-accent-foreground/20"
              >
                <Zap className="h-4 w-4" />
                Quick Start
              </Link>
            </div>

            {/* Tech Stack */}
            <div className="mt-16 flex items-center justify-center gap-6 text-sm text-fd-muted-foreground">
              <span className="flex items-center gap-2">
                <span className="h-1.5 w-1.5 rounded-full bg-blue-500" />
                C++
              </span>
              <span className="flex items-center gap-2">
                <span className="h-1.5 w-1.5 rounded-full bg-orange-500" />
                ImGui
              </span>
              <span className="flex items-center gap-2">
                <span className="h-1.5 w-1.5 rounded-full bg-green-500" />
                ImNodes
              </span>
            </div>
          </div>
        </div>
      </section>

      {/* Features Section */}
      <section className="border-b border-fd-border bg-fd-card/30">
        <div className="mx-auto max-w-6xl px-6 py-24">
          <div className="text-center mb-16">
            <h2 className="text-3xl font-bold text-fd-foreground sm:text-4xl">
              Everything you need
            </h2>
            <p className="mt-4 text-lg text-fd-muted-foreground max-w-2xl mx-auto">
              From simple gates to complex circuits, all in one intuitive interface.
            </p>
          </div>

          <div className="grid gap-6 sm:grid-cols-2 lg:grid-cols-3">
            <FeatureCard
              icon={<Boxes className="h-6 w-6" />}
              title="Visual Node Editor"
              description="Drag, drop, and connect logic gates on an intuitive canvas with real-time simulation."
            />
            <FeatureCard
              icon={<Code2 className="h-6 w-6" />}
              title="Script-Based DSL"
              description="Define circuits in code with bidirectional sync between script and visual graph."
            />
            <FeatureCard
              icon={<Database className="h-6 w-6" />}
              title="Gate Libraries"
              description="Create reusable custom gates and save them to libraries for any project."
            />
            <FeatureCard
              icon={<Zap className="h-6 w-6" />}
              title="Real-time Simulation"
              description="Watch signals propagate instantly. Toggle inputs and see outputs change immediately."
            />
            <FeatureCard
              icon={<Layers className="h-6 w-6" />}
              title="Hierarchical Design"
              description="Nest gates within gates for unlimited complexity. Build a CPU from basic gates."
            />
            <FeatureCard
              icon={<Cpu className="h-6 w-6" />}
              title="Native Performance"
              description="Built with C++ and ImGui for smooth experience even with complex circuits."
            />
          </div>
        </div>
      </section>

      {/* Code Example Section */}
      <section className="border-b border-fd-border">
        <div className="mx-auto max-w-6xl px-6 py-24">
          <div className="grid lg:grid-cols-2 gap-12 items-center">
            <div>
              <h2 className="text-3xl font-bold text-fd-foreground sm:text-4xl">
                Simple yet powerful
              </h2>
              <p className="mt-4 text-lg text-fd-muted-foreground">
                Define any circuit with a clean, readable syntax. Build complex systems from just AND and NOT gates.
              </p>

              <div className="mt-8 space-y-4">
                <div className="flex items-start gap-3">
                  <div className="flex h-6 w-6 shrink-0 items-center justify-center rounded-full bg-fd-primary/10 text-fd-primary text-sm font-medium">1</div>
                  <p className="text-fd-muted-foreground">Define nodes with position</p>
                </div>
                <div className="flex items-start gap-3">
                  <div className="flex h-6 w-6 shrink-0 items-center justify-center rounded-full bg-fd-primary/10 text-fd-primary text-sm font-medium">2</div>
                  <p className="text-fd-muted-foreground">Connect with arrow syntax</p>
                </div>
                <div className="flex items-start gap-3">
                  <div className="flex h-6 w-6 shrink-0 items-center justify-center rounded-full bg-fd-primary/10 text-fd-primary text-sm font-medium">3</div>
                  <p className="text-fd-muted-foreground">Create reusable custom gates</p>
                </div>
              </div>

              <Link
                href="/docs/dsl-reference"
                className="mt-8 inline-flex items-center gap-2 text-fd-primary hover:underline font-medium"
              >
                Learn the DSL
                <ArrowRight className="h-4 w-4" />
              </Link>
            </div>

            <div className="rounded-xl border border-fd-border bg-fd-card p-6 font-mono text-sm">
              <div className="flex items-center gap-2 mb-4 text-fd-muted-foreground">
                <div className="h-3 w-3 rounded-full bg-red-500/80" />
                <div className="h-3 w-3 rounded-full bg-yellow-500/80" />
                <div className="h-3 w-3 rounded-full bg-green-500/80" />
                <span className="ml-2 text-xs">circuit.bps</span>
              </div>
              <pre className="text-fd-muted-foreground overflow-x-auto">
                <code>{`// Define an XOR gate from primitives
define XOR(a, b) -> (out):
  na = NOT a
  nb = NOT b
  t1 = a AND nb
  t2 = na AND b
  out = OR(t1, t2)
end

// Use it in a circuit
In A @ 100, 100
In B @ 100, 200
XOR gate @ 250, 150
Out LED @ 400, 150

A -> gate.in0
B -> gate.in1
gate -> LED`}</code>
              </pre>
            </div>
          </div>
        </div>
      </section>

      {/* CTA Section */}
      <section className="border-b border-fd-border">
        <div className="mx-auto max-w-6xl px-6 py-24">
          <div className="relative overflow-hidden rounded-3xl border border-fd-border bg-gradient-to-br from-fd-card via-fd-card to-fd-primary/5 p-12 text-center">
            <div className="absolute inset-0 bg-[radial-gradient(circle_at_bottom_right,rgba(var(--primary-rgb),0.1),transparent_70%)]" />
            <div className="relative">
              <h2 className="text-3xl font-bold text-fd-foreground sm:text-4xl">
                Ready to build?
              </h2>
              <p className="mt-4 text-lg text-fd-muted-foreground max-w-xl mx-auto">
                Start with our tutorial and create your first working logic circuit in minutes.
              </p>
              <Link
                href="/docs/tutorial"
                className="mt-8 inline-flex items-center gap-2 rounded-xl bg-fd-primary px-8 py-4 text-sm font-semibold text-fd-primary-foreground shadow-lg shadow-fd-primary/25 transition-all hover:bg-fd-primary/90 hover:shadow-xl hover:-translate-y-0.5"
              >
                Start the Tutorial
                <ArrowRight className="h-4 w-4" />
              </Link>
            </div>
          </div>
        </div>
      </section>

      {/* Footer */}
      <footer className="bg-fd-card/30">
        <div className="mx-auto max-w-6xl px-6 py-12">
          <div className="flex flex-col items-center justify-between gap-6 sm:flex-row">
            <div className="flex items-center gap-2 text-fd-muted-foreground">
              <CircuitBoard className="h-5 w-5" />
              <span className="font-medium">Billyprints</span>
            </div>
            <nav className="flex flex-wrap items-center justify-center gap-6 text-sm">
              <Link href="/docs" className="text-fd-muted-foreground hover:text-fd-foreground transition-colors">
                Documentation
              </Link>
              <Link href="/docs/examples" className="text-fd-muted-foreground hover:text-fd-foreground transition-colors">
                Examples
              </Link>
              <Link href="/docs/faq" className="text-fd-muted-foreground hover:text-fd-foreground transition-colors">
                FAQ
              </Link>
              <a href="https://github.com" className="text-fd-muted-foreground hover:text-fd-foreground transition-colors">
                GitHub
              </a>
            </nav>
          </div>
        </div>
      </footer>
    </main>
  );
}

function FeatureCard({
  icon,
  title,
  description,
}: {
  icon: React.ReactNode;
  title: string;
  description: string;
}) {
  return (
    <div className="group rounded-2xl border border-fd-border bg-fd-card p-6 transition-all hover:border-fd-primary/30 hover:shadow-lg hover:shadow-fd-primary/5 hover:-translate-y-1">
      <div className="mb-4 inline-flex h-12 w-12 items-center justify-center rounded-xl bg-fd-primary/10 text-fd-primary transition-colors group-hover:bg-fd-primary group-hover:text-fd-primary-foreground">
        {icon}
      </div>
      <h3 className="text-lg font-semibold text-fd-foreground">{title}</h3>
      <p className="mt-2 text-sm text-fd-muted-foreground leading-relaxed">{description}</p>
    </div>
  );
}
