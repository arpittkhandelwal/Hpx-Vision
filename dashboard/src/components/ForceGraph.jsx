import React, { useEffect, useRef } from 'react';
import * as d3 from 'd3';

const ForceGraph = ({ data }) => {
  const svgRef = useRef();
  const simulationRef = useRef();

  useEffect(() => {
    if (!data) return;

    const width = 800;
    const height = 600;
    const svg = d3.select(svgRef.current);

    if (!simulationRef.current) {
      // First time setup
      svg.attr('viewBox', [0, 0, width, height]);
      
      // Arrow marker
      svg.append('defs').append('marker')
        .attr('id', 'arrowhead')
        .attr('viewBox', '-0 -5 10 10')
        .attr('refX', 20)
        .attr('refY', 0)
        .attr('orient', 'auto')
        .attr('markerWidth', 6)
        .attr('markerHeight', 6)
        .append('svg:path')
        .attr('d', 'M 0,-5 L 10 ,0 L 0,5')
        .attr('fill', '#666');

      simulationRef.current = d3.forceSimulation()
        .force('link', d3.forceLink().id(d => d.id).distance(100))
        .force('charge', d3.forceManyBody().strength(-300))
        .force('center', d3.forceCenter(width / 2, height / 2));

      svg.append('g').attr('class', 'links');
      svg.append('g').attr('class', 'nodes');
    }

    const simulation = simulationRef.current;
    const nodes = data.nodes.map(d => ({ ...d }));
    const links = data.links.map(d => ({ ...d }));

    // Update simulation
    simulation.nodes(nodes);
    simulation.force('link').links(links);
    simulation.alpha(0.3).restart();

    const link = svg.select('.links')
      .selectAll('line')
      .data(links, d => `${d.source}-${d.target}`)
      .join(
        enter => enter.append('line')
          .attr('class', 'link')
          .attr('marker-end', 'url(#arrowhead)')
          .style('opacity', 0)
          .call(enter => enter.transition().duration(500).style('opacity', 0.4)),
        update => update,
        exit => exit.transition().duration(500).style('opacity', 0).remove()
      );

    const clusterColors = ['#ff7e33', '#ef4444', '#f59e0b', '#ec4899', '#8b5cf6'];

    const node = svg.select('.nodes')
      .selectAll('g.node-group')
      .data(nodes, d => d.id)
      .join(
        enter => {
          const g = enter.append('g')
            .attr('class', 'node-group')
            .style('opacity', 0)
            .call(drag(simulation));
          
          g.append('circle').attr('r', 8);
          g.append('text')
            .attr('x', 12).attr('y', 4)
            .attr('font-size', '10px')
            .attr('fill', '#a1a1aa')
            .style('pointer-events', 'none');

          g.transition().duration(500).style('opacity', 1);
          return g;
        },
        update => update,
        exit => exit.transition().duration(500).style('opacity', 0).remove()
      );

    node.select('circle')
      .attr('fill', d => {
        if (!d.stalled) return '#3b82f6';
        const clusterIdx = (data.clusters || []).findIndex(c => c.includes(d.id));
        return clusterIdx !== -1 ? clusterColors[clusterIdx % clusterColors.length] : '#991b1b';
      })
      .attr('class', d => d.stalled ? 'pulse alert-node' : '');

    node.select('text').text(d => d.id);

    simulation.on('tick', () => {
      link
        .attr('x1', d => d.source.x)
        .attr('y1', d => d.source.y)
        .attr('x2', d => d.target.x)
        .attr('y2', d => d.target.y);

      node.attr('transform', d => `translate(${d.x},${d.y})`);
    });

    function drag(simulation) {
      return d3.drag()
        .on('start', (event) => {
          if (!event.active) simulation.alphaTarget(0.3).restart();
          event.subject.fx = event.subject.x;
          event.subject.fy = event.subject.y;
        })
        .on('drag', (event) => {
          event.subject.fx = event.x;
          event.subject.fy = event.y;
        })
        .on('end', (event) => {
          if (!event.active) simulation.alphaTarget(0);
          event.subject.fx = null;
          event.subject.fy = null;
        });
    }
  }, [data]);

  return <svg ref={svgRef} style={{ width: '100%', height: '100%' }} />;
};

export default ForceGraph;
